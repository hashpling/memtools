.PHONY: memlib memlib_cleanall

STEM := memlib
TGT_TYPE := lib
memlib_SOURCES := memorymap.cc memorydiff.cc

memlib_CPPFLAGS := -DPROJ=memlib
memlib_XINCLUDES := $(Dir)

include rules.mk

memlib_cleanall:

memlib:	$(TARGET)
