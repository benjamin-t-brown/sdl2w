# Compile sdl2w module interfaces into ./gcm.cache (run from the consumer dir).
#
#   make -f path/to/sdl2w/modules/make/build-bmi.mk \
#     SDL2W_MOD=path/to/sdl2w/modules BMIN_MOD=path/to/sdl2w/modules/bmin

SDL2W_MOD ?= .
BMIN_MOD ?= $(SDL2W_MOD)/bmin
include $(dir $(lastword $(MAKEFILE_LIST)))config.mk

OBJDIR = .sdl2w-bmi
SDL2W_CACHE_KEY := $(shell sh $(BMIN_MOD)/make/cache-key.sh "$(CXX)" "$(subst ",\",$(SDL2W_MODULE_CXXFLAGS) $(SDL2W_MODULE_INTERFACE_FLAGS) -I$(SDL2W_MOD) -I$(BMIN_MOD))")
SDL2W_CACHE_STAMP := gcm.cache/.sdl2w-config-$(SDL2W_CACHE_KEY)
SDL2W_READY_STAMP := gcm.cache/.sdl2w-ready-$(SDL2W_CACHE_KEY)
SDL2W_MODULE_ROOT := $(SDL2W_MOD)
SDL2W_MODULE_OBJDIR := $(OBJDIR)
SDL2W_MODULE_INCLUDES := -I$(SDL2W_MOD) -I$(BMIN_MOD)
SDL2W_MODULE_EXTRA_PREREQS := $(SDL2W_CACHE_STAMP)
.DEFAULT_GOAL := all
include $(dir $(lastword $(MAKEFILE_LIST)))native-rules.mk

.PHONY: all clean

all: $(SDL2W_INTERFACE_OBJECTS)
	@touch $(SDL2W_READY_STAMP)

$(SDL2W_CACHE_STAMP):
	rm -rf $(OBJDIR)
	@mkdir -p gcm.cache
	@touch $@

$(OBJDIR): | $(SDL2W_CACHE_STAMP)

clean:
	rm -rf $(OBJDIR) gcm.cache
