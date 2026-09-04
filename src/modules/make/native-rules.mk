# Reusable GCC rules for compiling SDL2W module interfaces.
#
# Required before including:
#   SDL2W_MODULE_ROOT       directory containing the .cppm sources
#   SDL2W_MODULE_OBJDIR     object output directory
#   SDL2W_MODULE_CXXFLAGS   compiler flags, including -fmodules-ts
# Optional:
#   SDL2W_MODULE_INCLUDES
#   SDL2W_MODULE_INTERFACE_FLAGS
#   SDL2W_MODULE_EXTRA_PREREQS (for example the prepared bmin BMI stamp)

include $(dir $(lastword $(MAKEFILE_LIST)))module-graph.mk

SDL2W_INTERFACE_OBJECTS := $(patsubst %,$(SDL2W_MODULE_OBJDIR)/%.o,$(SDL2W_INTERFACE_MODULES))

$(SDL2W_MODULE_OBJDIR):
	@mkdir -p $@

define SDL2W_NATIVE_INTERFACE_RULE
$(SDL2W_MODULE_OBJDIR)/$(1).o: \
		$(SDL2W_MODULE_ROOT)/$(1).cppm \
		$(patsubst %,$(SDL2W_MODULE_OBJDIR)/%.o,$(SDL2W_DEPS_$(1))) \
		$(SDL2W_MODULE_EXTRA_PREREQS) | $(SDL2W_MODULE_OBJDIR)
	$$(CXX) $$(SDL2W_MODULE_CXXFLAGS) $$(SDL2W_MODULE_INTERFACE_FLAGS) \
		$$(SDL2W_MODULE_INCLUDES) -c $$< -o $$@
endef

$(foreach module,$(SDL2W_INTERFACE_MODULES),\
  $(eval $(call SDL2W_NATIVE_INTERFACE_RULE,$(module))))
