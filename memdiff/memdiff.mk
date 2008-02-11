.PHONY: memdiff memdiff_cleanall

STEM := memdiff
TGT_TYPE := cppexe
memdiff_SOURCES := memdiff.cc

memdiff_INCLUDES := $(memlib_XINCLUDES)
memdiff_LDLIBS := memlib

include rules.mk

ifneq ($(findstring second-expansion,$(.FEATURES)),)
$(TARGET): $$(memlib_TARGET)
else
$(TARGET): $(memlib_TARGET)
endif

memdiff_cleanall: memlib_cleanall

memdiff: $(TARGET)
