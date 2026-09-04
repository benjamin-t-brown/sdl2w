# Precompile sdl2w + bmin modules with em++/clang and compile them to .o.
# Run from the consumer directory (or src/modules when building the wasm archive).
#
#   make -f path/to/sdl2w/modules/make/build-bmi-em.mk \
#     SDL2W_MOD=path/to/sdl2w/modules BMIN_MOD=path/to/bmin/modules
#
# Clang BMIs are not portable; this must run with the same em++ that links.

_SDL2W_EM_MAKE_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
include $(_SDL2W_EM_MAKE_DIR)/wasm-flags.mk
include $(_SDL2W_EM_MAKE_DIR)/module-graph.mk

ifeq ($(origin CXX),default)
  CXX := em++
endif
SDL2W_MOD ?= .
BMIN_MOD ?= $(SDL2W_MOD)/bmin
PCMDIR = pcm.cache
SDL2W_OBJDIR = .sdl2w-bmi
FLAGS = $(EMCC_CXXFLAGS) -fprebuilt-module-path=$(PCMDIR) -I$(SDL2W_MOD) -I$(BMIN_MOD)
BMIN_BUILD_MK = $(BMIN_MOD)/make/build-bmi-clang.mk
include $(BMIN_MOD)/make/module-graph.mk

BMIN_CACHE_KEY := $(shell sh $(BMIN_MOD)/make/cache-key.sh "$(CXX)" "$(subst ",\",$(FLAGS))")
BMIN_READY_STAMP := $(PCMDIR)/.bmin-ready-$(BMIN_CACHE_KEY)
SDL2W_CACHE_KEY := $(shell sh $(BMIN_MOD)/make/cache-key.sh "$(CXX)" "$(subst ",\",$(FLAGS))")
SDL2W_CACHE_STAMP := $(PCMDIR)/.sdl2w-config-$(SDL2W_CACHE_KEY)
SDL2W_READY_STAMP := $(PCMDIR)/.sdl2w-ready-$(SDL2W_CACHE_KEY)

BMIN_SOURCES := \
	$(addprefix $(BMIN_MOD)/,$(addsuffix .cppm,$(BMIN_INTERFACE_MODULES))) \
	$(addprefix $(BMIN_MOD)/,$(addsuffix .cpp,$(BMIN_IMPLEMENTATION_MODULES)))
SDL2W_PCMS := $(patsubst %,$(PCMDIR)/%.pcm,$(SDL2W_INTERFACE_MODULES))
SDL2W_OBJS := $(patsubst %,$(SDL2W_OBJDIR)/%.o,$(SDL2W_INTERFACE_MODULES)) \
	$(patsubst %,$(SDL2W_OBJDIR)/%-impl.o,$(SDL2W_IMPLEMENTATION_MODULES))

.DEFAULT_GOAL := all
.PHONY: all clean

all: $(SDL2W_OBJS)
	@touch $(SDL2W_READY_STAMP)

$(BMIN_READY_STAMP): $(BMIN_BUILD_MK) $(BMIN_SOURCES)
	$(MAKE) -f $(BMIN_BUILD_MK) \
		CXX="$(CXX)" \
		BMIN_MOD=$(BMIN_MOD) \
		BMIN_PCMDIR=$(PCMDIR) \
		BMIN_OBJDIR=.bmin-bmi \
		BMIN_CLANG_CXXFLAGS="$(subst ",\",$(FLAGS))"

$(SDL2W_CACHE_STAMP): $(BMIN_READY_STAMP)
	rm -rf $(SDL2W_OBJDIR)
	@mkdir -p $(PCMDIR)
	@touch $@

$(SDL2W_OBJDIR): | $(SDL2W_CACHE_STAMP)
	@mkdir -p $@

# --- sdl2w precompile ---

define SDL2W_EM_PCM_RULE
$(PCMDIR)/$(1).pcm: \
		$(SDL2W_MOD)/$(1).cppm \
		$(patsubst %,$(PCMDIR)/%.pcm,$(SDL2W_DEPS_$(1))) \
		$(SDL2W_CACHE_STAMP)
	$$(CXX) $$(FLAGS) --precompile $$< -o $$@
endef

$(foreach module,$(SDL2W_INTERFACE_MODULES),\
  $(eval $(call SDL2W_EM_PCM_RULE,$(module))))

# --- pcm / impl -> object ---

$(SDL2W_OBJDIR)/%.o: $(PCMDIR)/%.pcm | $(SDL2W_OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(SDL2W_OBJDIR)/%-impl.o: $(SDL2W_MOD)/%.cpp $(PCMDIR)/%.pcm | $(SDL2W_OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(PCMDIR) .bmin-bmi $(SDL2W_OBJDIR)
