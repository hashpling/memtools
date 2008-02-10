.PHONY: splitrec splitrec_cleanall

STEM := splitrec
TGT_TYPE := cppexe
splitrec_SOURCES := splitrec.cc

splitrec_INCLUDES := $(memlib_XINCLUDES)
splitrec_LDLIBS := memlib

include rules.mk

TEST_FILES += $(TARGET)

ifneq ($(findstring second-expansion,$(.FEATURES)),)
$(TARGET): $$(memlib_TARGET)
else
$(TARGET): $(memlib_TARGET)
endif

splitrec_cleanall: memlib_cleanall

splitrec: $(TARGET)
