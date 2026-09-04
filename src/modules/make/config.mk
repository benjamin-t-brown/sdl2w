# Shared compiler settings for SDL2W named modules.

SDL2W_HOST_OS := $(shell uname -s 2>/dev/null)

# /usr/bin/g++ is Apple Clang. The native module build currently targets the
# GCC 15 toolchain also used by the bundled bmin modules.
ifeq ($(SDL2W_HOST_OS),Darwin)
  ifeq ($(origin CXX),default)
    CXX := g++-15
  endif
  SDL2W_MODULE_PLATFORM_CXXFLAGS := -I/opt/homebrew/include -I/usr/local/include
endif

SDL2W_MODULE_CXXFLAGS ?= -Wall -std=c++23 -g -fmodules-ts $(SDL2W_MODULE_PLATFORM_CXXFLAGS)
SDL2W_MODULE_INTERFACE_FLAGS ?= -x c++
