.PHONY: memcat memcat_cleanall

STEM := memcat
TGT_TYPE := cppexe
memcat_SOURCES := memcat.cc

memcat_INCLUDES := $(memlib_XINCLUDES)
memcat_LDLIBS := memlib

include rules.mk

ifneq ($(findstring second-expansion,$(.FEATURES)),)
$(TARGET): $$(memlib_TARGET)
else
$(TARGET): $(memlib_TARGET)
endif

memcat_cleanall: memlib_cleanall

memcat: $(TARGET)
