#!/usr/bin/env bash

SCRIPT_NAME=$(basename "$0")
VERSION="1.0.0"
DEPENDENCIES=(curl bc security stty)

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'



if [[ -n "${NO_COLOR:-}" ]] || [[ "${TERM:-}" == "dumb" ]]; then
    RED="" GREEN="" YELLOW="" BLUE="" NC=""
fi

KEYCHAIN_SERVICE="Claude Code-credentials"
DEFAULT_BAUD=115200
DEFAULT_INTERVAL=300

function usage() {
    cat <<EOM

Reads Claude token usage from Anthropic rate-limit headers and pushes
a compact JSON payload to a Pico Display over USB serial.

usage: ${SCRIPT_NAME} [options]

options:
    -p|--port         <device>   Serial port (e.g. /dev/cu.usbmodem1234)
                                  Default: first /dev/cu.usbmodem* found
    -i|--interval     <seconds>  Polling interval in seconds (default: ${DEFAULT_INTERVAL})
    -b|--baud         <rate>     Baud rate (default: ${DEFAULT_BAUD})
    --version                    Show version information
    -h|--help                    Show this help message

dependencies: ${DEPENDENCIES[*]}

examples:
    ${SCRIPT_NAME}
    ${SCRIPT_NAME} -p /dev/cu.usbmodem1234
    ${SCRIPT_NAME} -p /dev/cu.usbmodem1234 -i 60

EOM
    exit 1
}

function main() {
    local serial_port=""
    local interval="$DEFAULT_INTERVAL"
    local baud="$DEFAULT_BAUD"

    while [[ $# -gt 0 ]]; do
        case "$1" in
        -p | --port)
            shift
            serial_port="$1"
            ;;
        -i | --interval)
            shift
            interval="$1"
            ;;
        -b | --baud)
            shift
            baud="$1"
            ;;
        --version)
            echo "${SCRIPT_NAME} version ${VERSION}"
            exit 0
            ;;
        -h | --help)
            usage
            ;;
        *)
            echo "Error: Unknown option '$1'" >&2
            usage
            ;;
        esac
        shift
    done

    exit_on_missing_tools "${DEPENDENCIES[@]}"

    local probe
    probe=$(read_token)
    if [[ -z "$probe" ]]; then
        print_error "Could not read Claude token (checked Keychain service \"${KEYCHAIN_SERVICE}\" and ~/.claude/.credentials.json)"
        exit 1
    fi

    if [[ -z "$serial_port" ]]; then
        serial_port=$(find_pico_port)
        if [[ -z "$serial_port" ]]; then
            print_error "No Pico USB serial port found (/dev/cu.usbmodem*)"
            exit 1
        fi
    fi

    echo -e "${BLUE}Serial port : ${NC}${serial_port}"
    echo -e "${BLUE}Interval    : ${NC}${interval}s"

    stty -f "$serial_port" "$baud" cs8 -cstopb -parenb raw 2>/dev/null \
        || print_warning "stty failed (port may still work)"

    # Open the port once (read+write) so DTR stays asserted for the Pico SDK
    # stdio_usb_connected() requires a persistent host connection to pass data.
    exec 3<>"$serial_port" || { print_error "Cannot open serial port $serial_port"; exit 1; }

    run_loop "$interval"
}

function send_to_pico() {
    printf '%s\n' "$1" >&3 2>/dev/null
}

function run_loop() {
    local interval="$1"

    while true; do
        local token
        token=$(read_token)
        if [[ -z "$token" ]]; then
            print_warning "Empty token, retrying in ${interval}s"
            sleep "$interval"
            continue
        fi

        local json
        if json=$(fetch_usage "$token"); then
            echo -e "${GREEN}→${NC} $json"
            if ! send_to_pico "$json"; then
                print_warning "Write to serial port failed"
            fi
        else
            print_warning "Fetch failed, retrying in ${interval}s"
        fi

        sleep "$interval"
    done
}

function read_token() {
    local raw
    raw=$(security find-generic-password -s "$KEYCHAIN_SERVICE" -w 2>/dev/null)
    if [[ -z "$raw" ]]; then
        raw=$(security find-internet-password -s "$KEYCHAIN_SERVICE" -w 2>/dev/null)
    fi

    if [[ -n "$raw" ]]; then
        echo "$raw" | grep -o '"accessToken":"[^"]*"' | cut -d'"' -f4
        return
    fi

    local creds_file="$HOME/.claude/.credentials.json"
    if [[ -f "$creds_file" ]]; then
        grep -o '"accessToken":"[^"]*"' "$creds_file" | cut -d'"' -f4
    fi
}

function fetch_usage() {
    local token="$1"
    local raw_headers http_status
    local session_utilisation_raw session_reset_raw weekly_utilisation_raw weekly_reset_raw status_raw
    local session_percentage weekly_percentage session_reset_minutes weekly_reset_minutes rate_status
    local -r UNKNOWN_RESET=-1

    raw_headers=$(curl -s -D - -o /dev/null \
        "https://api.anthropic.com/v1/messages" \
        -H "Authorization: Bearer $token" \
        -H "Content-Type: application/json" \
        -H "anthropic-version: 2023-06-01" \
        -H "anthropic-beta: claude-code-20250219" \
        --max-time 15 \
        -d '{"model":"claude-haiku-4-5-20251001","max_tokens":1,"messages":[{"role":"user","content":"hi"}]}') || return 1

    http_status=$(echo "$raw_headers" | grep -m1 "^HTTP" | awk '{print $2}')
    if [[ "$http_status" != "200" ]]; then
        print_warning "API returned HTTP ${http_status} (token may be expired)"
        return 1
    fi

    session_utilisation_raw=$(extract_header "$raw_headers" "anthropic-ratelimit-unified-5h-utilization")
    session_reset_raw=$(extract_header       "$raw_headers" "anthropic-ratelimit-unified-5h-reset")
    weekly_utilisation_raw=$(extract_header  "$raw_headers" "anthropic-ratelimit-unified-7d-utilization")
    weekly_reset_raw=$(extract_header        "$raw_headers" "anthropic-ratelimit-unified-7d-reset")
    status_raw=$(extract_header              "$raw_headers" "anthropic-ratelimit-unified-5h-status")

    session_percentage=$(to_pct "$session_utilisation_raw")
    weekly_percentage=$(to_pct  "$weekly_utilisation_raw")

    if [[ -n "$session_reset_raw" ]]; then
        session_reset_minutes=$(iso_to_mins_from_now "$session_reset_raw")
    else
        session_reset_minutes=$UNKNOWN_RESET
    fi
    if [[ -n "$weekly_reset_raw" ]]; then
        weekly_reset_minutes=$(iso_to_mins_from_now "$weekly_reset_raw")
    else
        weekly_reset_minutes=$UNKNOWN_RESET
    fi

    rate_status="${status_raw:-unknown}"

    printf '{"session_percentage":%d,"session_reset_minutes":%d,"weekly_percentage":%d,"weekly_reset_minutes":%d,"status":"%s","fetch_succeeded":true}\n' \
        "$session_percentage" "$session_reset_minutes" "$weekly_percentage" "$weekly_reset_minutes" "$rate_status"
}

function find_pico_port() {
    ls /dev/cu.usbmodem* 2>/dev/null | head -1
}

# Accepts Unix timestamp (integer) or ISO 8601 date; outputs minutes until reset, or -1 on parse failure.
function iso_to_mins_from_now() {
    local ts="$1"
    local epoch_then epoch_now diff
    if [[ "$ts" =~ ^[0-9]+$ ]]; then
        epoch_then="$ts"
    else
        epoch_then=$(date -j -f "%Y-%m-%dT%H:%M:%SZ" "$ts" "+%s" 2>/dev/null) || { echo -1; return; }
    fi
    epoch_now=$(date +%s)
    diff=$(( (epoch_then - epoch_now + 59) / 60 ))
    (( diff < 0 )) && diff=0
    echo "$diff"
}

function extract_header() {
    local headers="$1"
    local name="$2"
    echo "$headers" \
        | grep -i "^${name}:" \
        | head -1 \
        | awk '{print $2}' \
        | tr -d '\r\n'
}

function to_pct() {
    local v="${1:-0}"
    local result
    result=$(echo "$v * 100" | bc -l 2>/dev/null) || result=0
    printf "%.0f" "$result"
}

function exit_on_missing_tools() {
    for cmd in "$@"; do
        if command -v "$cmd" &>/dev/null; then
            continue
        fi
        printf "Error: Required tool '%s' is not installed or not in PATH\n" "$cmd" >&2
        exit 1
    done
}

function print_error() {
    echo -e "${RED}Error: $*${NC}" >&2
}

function print_warning() {
    echo -e "${YELLOW}Warning: $*${NC}"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
    exit 0
fi