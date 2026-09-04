# Native tools against the modules API.

ifeq ($(OS),Windows_NT)
  EXE_SUFFIX = .exe
else
  EXE_SUFFIX =
endif

.PHONY: all clean Anims L10nScanner
.DEFAULT_GOAL := all

include ../modules/make/use.mk

OUT = ../build/tools

all: $(OUT)/Anims$(EXE_SUFFIX) $(OUT)/L10nScanner$(EXE_SUFFIX)

$(OUT):
	@mkdir -p $@

Anims.o: Anims.cpp | sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -c Anims.cpp -o $@

$(OUT)/Anims$(EXE_SUFFIX): Anims.o $(SDL2W_LIB) | $(OUT)
	$(CXX) $(SDL2W_CXXFLAGS) -o $@ Anims.o $(SDL2W_LDLIBS)

$(OUT)/L10nScanner$(EXE_SUFFIX): L10nScanner.cpp | $(OUT)
	$(CXX) -Wall -std=c++23 -o $@ L10nScanner.cpp

Anims: $(OUT)/Anims$(EXE_SUFFIX)
L10nScanner: $(OUT)/L10nScanner$(EXE_SUFFIX)

clean:
	rm -f Anims.o
	rm -rf $(OUT)
	rm -rf gcm.cache .bmin-bmi .sdl2w-bmi
