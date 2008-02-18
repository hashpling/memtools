.PHONY: joinrec joinrec_cleanall

STEM := joinrec
TGT_TYPE := cppexe
joinrec_SOURCES := joinrec.cc

joinrec_INCLUDES := $(memlib_XINCLUDES)
joinrec_LDLIBS := memlib

include rules.mk

ifneq ($(findstring second-expansion,$(.FEATURES)),)
$(TARGET): $$(memlib_TARGET)
else
$(TARGET): $(memlib_TARGET)
endif

joinrec_cleanall: memlib_cleanall

joinrec: $(TARGET)
