PICO_SDK_PATH    := $(CURDIR)/lib/pico-sdk
PIMORONI_PATH    := $(CURDIR)/lib/pimoroni-pico
BUILD_DIR        := build
TEST_BUILD_DIR   := build-tests

LLVM_PREFIX      := $(shell brew --prefix llvm 2>/dev/null)
CLANG_FORMAT     := $(if $(LLVM_PREFIX),$(LLVM_PREFIX)/bin/clang-format,clang-format)
CLANG_TIDY       := $(if $(LLVM_PREFIX),$(LLVM_PREFIX)/bin/clang-tidy,clang-tidy)
SHELLCHECK       := shellcheck

ARM_SYSROOT     := $(shell arm-none-eabi-g++ -print-sysroot 2>/dev/null)
ARM_GCC_VER     := $(shell arm-none-eabi-g++ -dumpversion 2>/dev/null)
ARM_GCC_LIBDIR  := $(dir $(shell arm-none-eabi-g++ -print-libgcc-file-name 2>/dev/null))

SOURCES_CPP    := $(shell find src -name '*.cpp') main.cpp
SOURCES_ALL    := $(shell find src -name '*.cpp' -o -name '*.hpp') main.cpp
SHELL_SCRIPTS  := $(shell find . -maxdepth 1 -name '*.sh')
DAEMON_SCRIPT  := $(CURDIR)/claude-usage-daemon.sh

DAEMON_PORT     ?=
DAEMON_INTERVAL ?= 300
DAEMON_BAUD     ?= 115200

.PHONY: all configure build test clean fmt fmt-check tidy shellcheck lint daemon

all: build

configure:
	rm -rf $(BUILD_DIR)
	cmake -B $(BUILD_DIR) -S . \
		-G Ninja \
		-DPICO_SDK_PATH="$(PICO_SDK_PATH)" \
		-DPIMORONI_PICO_PATH="$(PIMORONI_PATH)" \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json

build: configure
	ninja -C $(BUILD_DIR)

test:
	cmake -B $(TEST_BUILD_DIR) -S tests -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(TEST_BUILD_DIR)
	ctest --test-dir $(TEST_BUILD_DIR) --output-on-failure

fmt:
	$(CLANG_FORMAT) -i $(SOURCES_ALL)

fmt-check:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES_ALL)

tidy:
	$(CLANG_TIDY) -p . $(SOURCES_CPP) \
		--extra-arg=-I$(ARM_SYSROOT)/include/c++/$(ARM_GCC_VER) \
		--extra-arg=-I$(ARM_SYSROOT)/include/c++/$(ARM_GCC_VER)/arm-none-eabi \
		--extra-arg=-I$(ARM_SYSROOT)/include \
		--extra-arg=-I$(ARM_GCC_LIBDIR)include \
		--extra-arg=-I$(ARM_GCC_LIBDIR)include-fixed \
		--extra-arg=-Wno-invalid-constexpr

shellcheck:
	$(SHELLCHECK) --severity=warning $(SHELL_SCRIPTS)

lint: fmt-check tidy shellcheck

daemon:
	bash $(DAEMON_SCRIPT) \
		$(if $(DAEMON_PORT),--port $(DAEMON_PORT)) \
		--interval $(DAEMON_INTERVAL) \
		--baud $(DAEMON_BAUD)

clean:
	rm -rf $(BUILD_DIR) $(TEST_BUILD_DIR) cmake-build-debug compile_commands.json