# Compile sdl2w module interfaces into ./gcm.cache (run from the consumer dir).
#
#   make -f path/to/sdl2w/modules/make/build-bmi.mk \
#     SDL2W_MOD=path/to/sdl2w/modules BMIN_MOD=path/to/sdl2w/modules/bmin

CXX ?= g++
SDL2W_MOD ?= .
BMIN_MOD ?= $(SDL2W_MOD)/bmin
FLAGS = -Wall -std=c++23 -g -fmodules-ts -I$(SDL2W_MOD) -I$(BMIN_MOD)
OBJDIR = .sdl2w-bmi

.PHONY: all clean

all: \
	$(OBJDIR)/sdl2w.defines.o \
	$(OBJDIR)/sdl2w.logger.o \
	$(OBJDIR)/sdl2w.types.o \
	$(OBJDIR)/sdl2w.events.o \
	$(OBJDIR)/sdl2w.animation.o \
	$(OBJDIR)/sdl2w.store.o \
	$(OBJDIR)/sdl2w.draw.o \
	$(OBJDIR)/sdl2w.assets.o \
	$(OBJDIR)/sdl2w.l10n.o \
	$(OBJDIR)/sdl2w.emscripten.o \
	$(OBJDIR)/sdl2w.window.o \
	$(OBJDIR)/sdl2w.init.o \
	$(OBJDIR)/sdl2w.o
	@mkdir -p gcm.cache
	@touch gcm.cache/.sdl2w-ready

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/sdl2w.defines.o: $(SDL2W_MOD)/sdl2w.defines.cppm | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.logger.o: $(SDL2W_MOD)/sdl2w.logger.cppm $(OBJDIR)/sdl2w.defines.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.types.o: $(SDL2W_MOD)/sdl2w.types.cppm $(OBJDIR)/sdl2w.defines.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.events.o: $(SDL2W_MOD)/sdl2w.events.cppm $(OBJDIR)/sdl2w.logger.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.animation.o: $(SDL2W_MOD)/sdl2w.animation.cppm $(OBJDIR)/sdl2w.types.o $(OBJDIR)/sdl2w.logger.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.store.o: $(SDL2W_MOD)/sdl2w.store.cppm $(OBJDIR)/sdl2w.animation.o $(OBJDIR)/sdl2w.logger.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.draw.o: $(SDL2W_MOD)/sdl2w.draw.cppm $(OBJDIR)/sdl2w.animation.o $(OBJDIR)/sdl2w.store.o $(OBJDIR)/sdl2w.logger.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.assets.o: $(SDL2W_MOD)/sdl2w.assets.cppm $(OBJDIR)/sdl2w.draw.o $(OBJDIR)/sdl2w.store.o $(OBJDIR)/sdl2w.logger.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.l10n.o: $(SDL2W_MOD)/sdl2w.l10n.cppm $(OBJDIR)/sdl2w.assets.o $(OBJDIR)/sdl2w.logger.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.emscripten.o: $(SDL2W_MOD)/sdl2w.emscripten.cppm $(OBJDIR)/sdl2w.logger.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.window.o: $(SDL2W_MOD)/sdl2w.window.cppm \
		$(OBJDIR)/sdl2w.draw.o $(OBJDIR)/sdl2w.events.o $(OBJDIR)/sdl2w.store.o \
		$(OBJDIR)/sdl2w.assets.o $(OBJDIR)/sdl2w.emscripten.o $(OBJDIR)/sdl2w.logger.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.init.o: $(SDL2W_MOD)/sdl2w.init.cppm $(OBJDIR)/sdl2w.window.o $(OBJDIR)/sdl2w.l10n.o $(OBJDIR)/sdl2w.logger.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR)/sdl2w.o: $(SDL2W_MOD)/sdl2w.cppm \
		$(OBJDIR)/sdl2w.defines.o $(OBJDIR)/sdl2w.logger.o $(OBJDIR)/sdl2w.types.o \
		$(OBJDIR)/sdl2w.events.o $(OBJDIR)/sdl2w.animation.o $(OBJDIR)/sdl2w.store.o \
		$(OBJDIR)/sdl2w.draw.o $(OBJDIR)/sdl2w.assets.o $(OBJDIR)/sdl2w.l10n.o \
		$(OBJDIR)/sdl2w.window.o $(OBJDIR)/sdl2w.init.o $(OBJDIR)/sdl2w.emscripten.o | $(OBJDIR)
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) gcm.cache
