# Locate emsdk (sibling of the sdl2w repo by default: ../emsdk from repo root)
# and put em++ on PATH. Override with EMSDK=/path/to/emsdk.

_EMSDK_TRY := \
	$(EMSDK) \
	$(abspath $(CURDIR)/../../emsdk) \
	$(abspath $(CURDIR)/../emsdk) \
	$(abspath $(CURDIR)/../../../emsdk) \
	/c/progs/emsdk \
	C:/progs/emsdk

EMSDK := $(firstword $(foreach d,$(_EMSDK_TRY),$(if $(wildcard $(d)/upstream/emscripten),$(d),)))

ifeq ($(EMSDK),)
  $(error emsdk not found. Set EMSDK to the emsdk directory (expected sibling ../emsdk))
endif

export EMSDK
export EMSDK_NODE := $(firstword $(wildcard $(EMSDK)/node/*_64bit/bin/node.exe $(EMSDK)/node/*_64bit/node $(EMSDK)/node/*_64bit/bin/node))
export EMSDK_PYTHON := $(firstword $(wildcard $(EMSDK)/python/*_64bit/python.exe $(EMSDK)/python/*_64bit/bin/python3 $(EMSDK)/python/*_64bit/bin/python))
export EMSDK_QUIET := 1
export PATH := $(EMSDK):$(EMSDK)/upstream/emscripten:$(EMSDK)/upstream/bin:$(PATH)

CXX := em++
AR := emar
