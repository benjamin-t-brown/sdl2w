# Precompile sdl2w + bmin modules with em++/clang and compile them to .o.
# Run from the consumer directory (or src/modules when building the wasm archive).
#
#   make -f path/to/sdl2w/modules/make/build-bmi-em.mk \
#     SDL2W_MOD=path/to/sdl2w/modules BMIN_MOD=path/to/bmin/modules
#
# Clang BMIs are not portable; this must run with the same em++ that links.

_SDL2W_EM_MAKE_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
include $(_SDL2W_EM_MAKE_DIR)/wasm-flags.mk

CXX ?= em++
SDL2W_MOD ?= .
BMIN_MOD ?= $(SDL2W_MOD)/bmin
PCMDIR = pcm.cache
BMIN_OBJDIR = .bmin-bmi
SDL2W_OBJDIR = .sdl2w-bmi
FLAGS = $(EMCC_CXXFLAGS) -fprebuilt-module-path=$(PCMDIR) -I$(SDL2W_MOD) -I$(BMIN_MOD)

BMIN_NAMES = \
	bmin.types \
	bmin.detail \
	bmin.dynarray \
	bmin.unique_ptr \
	bmin.string \
	bmin.list \
	bmin.queue \
	bmin.hash \
	bmin.map \
	bmin.stringstream \
	bmin.string_interop \
	bmin.containers

SDL2W_IFACE_NAMES = \
	sdl2w.defines \
	sdl2w.logger \
	sdl2w.types \
	sdl2w.events \
	sdl2w.animation \
	sdl2w.store \
	sdl2w.draw \
	sdl2w.assets \
	sdl2w.l10n \
	sdl2w.emscripten \
	sdl2w.window \
	sdl2w.init
SDL2W_NAMES = $(SDL2W_IFACE_NAMES) sdl2w

BMIN_PCMS = $(patsubst %,$(PCMDIR)/%.pcm,$(BMIN_NAMES))
SDL2W_PCMS = $(patsubst %,$(PCMDIR)/%.pcm,$(SDL2W_NAMES))
BMIN_OBJS = $(patsubst %,$(BMIN_OBJDIR)/%.o,$(BMIN_NAMES)) \
	$(BMIN_OBJDIR)/bmin.detail-impl.o \
	$(BMIN_OBJDIR)/bmin.string-impl.o \
	$(BMIN_OBJDIR)/bmin.stringstream-impl.o \
	$(BMIN_OBJDIR)/bmin.string_interop-impl.o
SDL2W_OBJS = $(patsubst %,$(SDL2W_OBJDIR)/%.o,$(SDL2W_NAMES))

.PHONY: all clean

all: $(BMIN_OBJS) $(SDL2W_OBJS)
	@mkdir -p $(PCMDIR)
	@touch $(PCMDIR)/.bmin-ready
	@touch $(PCMDIR)/.sdl2w-ready

$(PCMDIR) $(BMIN_OBJDIR) $(SDL2W_OBJDIR):
	@mkdir -p $@

# --- bmin precompile ---

$(PCMDIR)/bmin.types.pcm: $(BMIN_MOD)/bmin.types.cppm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.detail.pcm: $(BMIN_MOD)/bmin.detail.cppm $(PCMDIR)/bmin.types.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.dynarray.pcm: $(BMIN_MOD)/bmin.dynarray.cppm $(PCMDIR)/bmin.detail.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.unique_ptr.pcm: $(BMIN_MOD)/bmin.unique_ptr.cppm $(PCMDIR)/bmin.detail.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.string.pcm: $(BMIN_MOD)/bmin.string.cppm $(PCMDIR)/bmin.dynarray.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.list.pcm: $(BMIN_MOD)/bmin.list.cppm $(PCMDIR)/bmin.detail.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.queue.pcm: $(BMIN_MOD)/bmin.queue.cppm $(PCMDIR)/bmin.dynarray.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.hash.pcm: $(BMIN_MOD)/bmin.hash.cppm $(PCMDIR)/bmin.string.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.map.pcm: $(BMIN_MOD)/bmin.map.cppm $(PCMDIR)/bmin.hash.pcm $(PCMDIR)/bmin.list.pcm $(PCMDIR)/bmin.dynarray.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.stringstream.pcm: $(BMIN_MOD)/bmin.stringstream.cppm $(PCMDIR)/bmin.string.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.string_interop.pcm: $(BMIN_MOD)/bmin.string_interop.cppm $(PCMDIR)/bmin.string.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/bmin.containers.pcm: $(BMIN_MOD)/bmin.containers.cppm \
		$(PCMDIR)/bmin.string.pcm $(PCMDIR)/bmin.unique_ptr.pcm \
		$(PCMDIR)/bmin.dynarray.pcm $(PCMDIR)/bmin.list.pcm \
		$(PCMDIR)/bmin.queue.pcm $(PCMDIR)/bmin.hash.pcm \
		$(PCMDIR)/bmin.map.pcm $(PCMDIR)/bmin.stringstream.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

# --- sdl2w precompile ---

$(PCMDIR)/sdl2w.defines.pcm: $(SDL2W_MOD)/sdl2w.defines.cppm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.logger.pcm: $(SDL2W_MOD)/sdl2w.logger.cppm $(PCMDIR)/sdl2w.defines.pcm \
		$(PCMDIR)/bmin.string.pcm $(PCMDIR)/bmin.stringstream.pcm \
		$(PCMDIR)/bmin.string_interop.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.types.pcm: $(SDL2W_MOD)/sdl2w.types.cppm $(PCMDIR)/sdl2w.defines.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.events.pcm: $(SDL2W_MOD)/sdl2w.events.cppm $(PCMDIR)/sdl2w.logger.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.animation.pcm: $(SDL2W_MOD)/sdl2w.animation.cppm $(PCMDIR)/sdl2w.types.pcm $(PCMDIR)/sdl2w.logger.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.store.pcm: $(SDL2W_MOD)/sdl2w.store.cppm $(PCMDIR)/sdl2w.animation.pcm $(PCMDIR)/sdl2w.logger.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.draw.pcm: $(SDL2W_MOD)/sdl2w.draw.cppm $(PCMDIR)/sdl2w.animation.pcm $(PCMDIR)/sdl2w.store.pcm $(PCMDIR)/sdl2w.logger.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.assets.pcm: $(SDL2W_MOD)/sdl2w.assets.cppm $(PCMDIR)/sdl2w.draw.pcm $(PCMDIR)/sdl2w.store.pcm $(PCMDIR)/sdl2w.logger.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.l10n.pcm: $(SDL2W_MOD)/sdl2w.l10n.cppm $(PCMDIR)/sdl2w.assets.pcm $(PCMDIR)/sdl2w.logger.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.emscripten.pcm: $(SDL2W_MOD)/sdl2w.emscripten.cppm $(PCMDIR)/sdl2w.logger.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.window.pcm: $(SDL2W_MOD)/sdl2w.window.cppm \
		$(PCMDIR)/sdl2w.draw.pcm $(PCMDIR)/sdl2w.events.pcm $(PCMDIR)/sdl2w.store.pcm \
		$(PCMDIR)/sdl2w.assets.pcm $(PCMDIR)/sdl2w.emscripten.pcm $(PCMDIR)/sdl2w.logger.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.init.pcm: $(SDL2W_MOD)/sdl2w.init.cppm $(PCMDIR)/sdl2w.window.pcm $(PCMDIR)/sdl2w.l10n.pcm $(PCMDIR)/sdl2w.logger.pcm | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

$(PCMDIR)/sdl2w.pcm: $(SDL2W_MOD)/sdl2w.cppm $(patsubst %,$(PCMDIR)/%.pcm,$(SDL2W_IFACE_NAMES)) | $(PCMDIR)
	$(CXX) $(FLAGS) --precompile $< -o $@

# --- pcm / impl -> object ---

$(BMIN_OBJDIR)/%.o: $(PCMDIR)/%.pcm | $(BMIN_OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(SDL2W_OBJDIR)/%.o: $(PCMDIR)/%.pcm | $(SDL2W_OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(BMIN_OBJDIR)/bmin.detail-impl.o: $(BMIN_MOD)/bmin.detail.cpp $(PCMDIR)/bmin.detail.pcm | $(BMIN_OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(BMIN_OBJDIR)/bmin.string-impl.o: $(BMIN_MOD)/bmin.string.cpp $(PCMDIR)/bmin.string.pcm | $(BMIN_OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(BMIN_OBJDIR)/bmin.stringstream-impl.o: $(BMIN_MOD)/bmin.stringstream.cpp $(PCMDIR)/bmin.stringstream.pcm | $(BMIN_OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(BMIN_OBJDIR)/bmin.string_interop-impl.o: $(BMIN_MOD)/bmin.string_interop.cpp $(PCMDIR)/bmin.string_interop.pcm | $(BMIN_OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(PCMDIR) $(BMIN_OBJDIR) $(SDL2W_OBJDIR)
