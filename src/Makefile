# sdl2w is C++ modules only. Native: GCC + -fmodules-ts.
# Wasm: em++ (Clang PCMs) via make wasm / example make js.

TARGET ?= native
.DEFAULT_GOAL := native

BMIN_URL ?= https://github.com/benjamin-t-brown/bmin.git
BMIN_REF ?= experiment/cpp-modules
BMIN_REPO ?= ../bmin
BMIN_SRC = $(BMIN_REPO)/src
BMIN_BUILD_OUT = $(BMIN_REPO)/bmin
BMIN_BUILT_MODULES_LIB = $(BMIN_BUILD_OUT)/lib/libbmin_modules.a
BMIN_BUILT_MODULES = $(BMIN_BUILD_OUT)/modules

DEPS_DIR = .
DEPS_BMIN_MODULES_LIB = $(DEPS_DIR)/bmin/libbmin_modules.a
DEPS_BMIN_MODULES_MARKER = $(DEPS_DIR)/bmin/modules/bmin.containers.cppm

INSTALL_DIR = ../sdl2w

.PHONY: all native wasm dist clean clean-deps bmin modules tools

$(BMIN_REPO)/.git:
	@mkdir -p $(dir $(BMIN_REPO))
	@if [ ! -d "$(BMIN_REPO)/.git" ]; then \
		echo "Cloning bmin from $(BMIN_URL) ($(BMIN_REF))..."; \
		git clone --depth 1 --branch $(BMIN_REF) $(BMIN_URL) $(BMIN_REPO); \
	fi

$(BMIN_BUILT_MODULES_LIB): $(BMIN_REPO)/.git
	@echo "Building bmin modules for $(TARGET)..."
	@$(MAKE) -C $(BMIN_SRC) all TARGET=$(TARGET)

$(DEPS_BMIN_MODULES_MARKER):
	@if [ ! -f "$(BMIN_BUILT_MODULES_LIB)" ]; then \
		$(MAKE) $(BMIN_BUILT_MODULES_LIB); \
	fi
	@if [ ! -f "$(BMIN_BUILT_MODULES_LIB)" ]; then \
		echo "error: $(BMIN_BUILT_MODULES_LIB) not found after bmin build" >&2; \
		exit 1; \
	fi
	@mkdir -p $(DEPS_DIR)/bmin/modules/make
	@cp $(BMIN_BUILT_MODULES_LIB) $(DEPS_DIR)/bmin/
	@cp -R $(BMIN_BUILT_MODULES)/* $(DEPS_DIR)/bmin/modules/

bmin: $(DEPS_BMIN_MODULES_MARKER)

native:
	@$(MAKE) all TARGET=native

wasm: $(DEPS_BMIN_MODULES_MARKER)
	@$(MAKE) -C modules lib TARGET=wasm
	@mkdir -p $(INSTALL_DIR)/lib/wasm
	@cp -f modules/build-wasm/libsdl2w_modules.a $(INSTALL_DIR)/lib/wasm/
	@cp -f modules/build-wasm/libbmin_modules.a $(INSTALL_DIR)/lib/wasm/
	@$(MAKE) -C modules install-files
	@echo "Created wasm sdl2w archives at $(INSTALL_DIR)/lib/wasm"

modules:
	@$(MAKE) -C modules all

tools: $(DEPS_BMIN_MODULES_MARKER)
	@$(MAKE) -C modules lib
	@$(MAKE) -C tools

all: dist

dist: $(DEPS_BMIN_MODULES_MARKER)
	@$(MAKE) -C modules install
	@rm -rf $(INSTALL_DIR)/include
	@rm -f $(INSTALL_DIR)/lib/libsdl2w.a $(INSTALL_DIR)/lib/libbmin.a
	@mkdir -p $(INSTALL_DIR)/lib
	@cp $(DEPS_BMIN_MODULES_LIB) $(INSTALL_DIR)/lib/libbmin_modules.a
	@mkdir -p $(INSTALL_DIR)/modules/bmin/make
	@cp -R $(DEPS_DIR)/bmin/modules/* $(INSTALL_DIR)/modules/bmin/
	@echo "Created modules-only sdl2w bundle at $(INSTALL_DIR)"

clean:
	@echo "Cleaning..."
	rm -rf $(DEPS_DIR)/bmin
	rm -rf ../sdl2w
	@if [ -f modules/Makefile ]; then $(MAKE) -C modules clean; fi
	@if [ -f "$(BMIN_SRC)/Makefile" ]; then $(MAKE) -C $(BMIN_SRC) clean; fi

clean-deps: clean
	@echo "Removing bmin clone at $(BMIN_REPO)..."
	rm -rf $(BMIN_REPO)
