.PHONY: memtest memtest_cleanall

STEM := memtest
TGT_TYPE := cppexe
memtest_SOURCES := memtest.cc

memtest_INCLUDES := $(memlib_XINCLUDES)
memtest_LDLIBS := memlib

include rules.mk

TEST_FILES += $(TARGET)

ifneq ($(findstring second-expansion,$(.FEATURES)),)
$(TARGET): $$(memlib_TARGET)
else
$(TARGET): $(memlib_TARGET)
endif

memtest_cleanall: mem_cleanall

memtest: $(TARGET)
