# Canonical SDL2W module graph.
#
# Keep module names and their direct SDL2W dependencies here. Repository,
# installed-consumer, and Emscripten builds all derive their ordering from this
# file so the graph cannot drift between build paths.

SDL2W_COMPONENT_MODULES := \
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

SDL2W_PRIMARY_MODULE := sdl2w
SDL2W_INTERFACE_MODULES := $(SDL2W_COMPONENT_MODULES) $(SDL2W_PRIMARY_MODULE)

SDL2W_IMPLEMENTATION_MODULES := \
	sdl2w.defines \
	sdl2w.logger \
	sdl2w.events \
	sdl2w.animation \
	sdl2w.store \
	sdl2w.draw \
	sdl2w.assets \
	sdl2w.l10n \
	sdl2w.emscripten \
	sdl2w.window \
	sdl2w.init

SDL2W_DEPS_sdl2w.types := sdl2w.defines
SDL2W_DEPS_sdl2w.events := sdl2w.defines sdl2w.logger
SDL2W_DEPS_sdl2w.animation := sdl2w.types sdl2w.logger
SDL2W_DEPS_sdl2w.store := \
	sdl2w.animation sdl2w.defines sdl2w.types sdl2w.logger
SDL2W_DEPS_sdl2w.draw := \
	sdl2w.animation sdl2w.store sdl2w.types sdl2w.defines sdl2w.logger
SDL2W_DEPS_sdl2w.assets := \
	sdl2w.draw sdl2w.store sdl2w.defines sdl2w.logger
SDL2W_DEPS_sdl2w.l10n := sdl2w.assets sdl2w.defines sdl2w.logger
SDL2W_DEPS_sdl2w.emscripten := sdl2w.logger
SDL2W_DEPS_sdl2w.window := \
	sdl2w.defines sdl2w.draw sdl2w.events sdl2w.store sdl2w.types \
	sdl2w.assets sdl2w.emscripten sdl2w.logger
SDL2W_DEPS_sdl2w.init := sdl2w.window sdl2w.l10n sdl2w.logger
SDL2W_DEPS_sdl2w := $(SDL2W_COMPONENT_MODULES)

SDL2W_BMIN_DEPS_sdl2w.logger := \
	bmin.string bmin.stringstream bmin.string_interop
SDL2W_BMIN_DEPS_sdl2w.types := bmin.string
SDL2W_BMIN_DEPS_sdl2w.events := bmin.containers bmin.string_interop
SDL2W_BMIN_DEPS_sdl2w.animation := bmin.containers bmin.string_interop
SDL2W_BMIN_DEPS_sdl2w.store := bmin.containers bmin.string_interop
SDL2W_BMIN_DEPS_sdl2w.draw := \
	bmin.containers bmin.string_interop bmin.stringstream
SDL2W_BMIN_DEPS_sdl2w.assets := bmin.containers bmin.string_interop
SDL2W_BMIN_DEPS_sdl2w.l10n := bmin.containers bmin.string_interop
SDL2W_BMIN_DEPS_sdl2w.emscripten := \
	bmin.string bmin.stringstream bmin.string_interop
SDL2W_BMIN_DEPS_sdl2w.window := bmin.containers bmin.string_interop
SDL2W_BMIN_DEPS_sdl2w.init := bmin.containers bmin.string_interop
SDL2W_BMIN_DEPS_sdl2w := bmin.containers
