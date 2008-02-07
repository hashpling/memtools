USE_LINK := n

TGT_IS_SO := n
TGT_IS_EXE := n
TGT_IS_LIB := n

ifeq ($(TGT_TYPE),so)
TGT_IS_SO := y
endif

ifeq ($(TGT_TYPE),cppso)
TGT_IS_SO := y
endif

ifeq ($(TGT_TYPE),exe)
TGT_IS_EXE := y
endif

ifeq ($(TGT_TYPE),cppexe)
TGT_IS_EXE := y
endif

ifeq ($(TGT_TYPE),lib)
	$(STEM)_TGT := lib$(STEM).a
	TARGET := $(Out)/lib/$($(STEM)_TGT)
$(TARGET):	$(Out)/lib/.touch

endif

ifeq ($(TGT_IS_SO),y)
	$(STEM)_TGT := lib$(STEM).so
	TARGET := $(Out)/lib/$($(STEM)_TGT)
	USE_LINK := y
$(TARGET):	$(Out)/lib/.touch

endif

ifeq ($(TGT_IS_EXE),y)
	$(STEM)_TGT := $(STEM)
	TARGET := $(Out)/bin/$($(STEM)_TGT)
	USE_LINK := y
$(TARGET): $(Out)/bin/.touch

endif

$(STEM)_TARGET := $(TARGET)

$(STEM)_objdir := $(Out)/obj/$(STEM)

$(STEM)_CSOURCES := $(filter %.c,$($(STEM)_SOURCES))
$(STEM)_CXXSOURCES := $(filter %.cc,$($(STEM)_SOURCES))
$(STEM)_ASMSOURCES := $(filter %.asm,$($(STEM)_SOURCES))
$(STEM)_OBJS := $($(STEM)_CSOURCES:%.c=$($(STEM)_objdir)/%.o) $($(STEM)_CXXSOURCES:%.cc=$($(STEM)_objdir)/%.o) $($(STEM)_ASMSOURCES:%.asm=$($(STEM)_objdir)/%.o)
$(STEM)_DEPS := $($(STEM)_CSOURCES:%.c=$($(STEM)_objdir)/%.d) $($(STEM)_CXXSOURCES:%.cc=$($(STEM)_objdir)/%.d) $($(STEM)_ASMSOURCES:%.asm=$($(STEM)_objdir)/%.d)
$(STEM)_DOS := $($(STEM)_OBJS) $($(STEM)_DEPS) $(TARGET)

$($(STEM)_objdir)/.touch:
	$(makedir)

$($(STEM)_objdir)/%.o:	$(Dir)/%.c
	$(compilec)

$($(STEM)_objdir)/%.o:	$(Dir)/%.cc
	$(compilecpp)

$($(STEM)_objdir)/%.o:	$(Dir)/%.asm
	$(assemble)

$($(STEM)_objdir)/%.d:	$(Dir)/%.c
	$(makecdep)

$($(STEM)_objdir)/%.d:	$(Dir)/%.cc
	$(makecppdep)

$($(STEM)_objdir)/%.d:	$(Dir)/%.asm
	$(makeasmdep)

$($(STEM)_DOS): stem := $(STEM)

$($(STEM)_DOS):	CFLAGS = $(GLBL_CFLAGS) $($(stem)_CFLAGS)
$($(STEM)_DOS):	CXXFLAGS = $(GLBL_CXXFLAGS) $($(stem)_CXXFLAGS)
$($(STEM)_DOS):	CPPFLAGS = $(GLBL_CPPFLAGS) $($(stem)_CPPFLAGS)
$($(STEM)_DOS):	INCLUDES = $(GLBL_INCLUDES) $($(stem)_INCLUDES)
$($(STEM)_DOS):	ASMFLAGS = $(GLBL_ASMFLAGS) $($(stem)_ASMFLAGS)

$($(STEM)_DOS): $($(STEM)_objdir)/.touch

$(TARGET):	LDLIBS = $(GLBL_LDLIBS) $($(stem)_LDLIBS)
$(TARGET):	LDPATH = $(GLBL_LDPATH) $($(stem)_LDPATH)
$(TARGET):	LDRPATH = $(GLBL_LDRPATH) $($(stem)_LDRPATH)
$(TARGET):	LDFLAGS = $(GLBL_LDFLAGS) $($(stem)_LDFLAGS)

$(TARGET):	OBJECTS := $($(STEM)_OBJS)

$(TARGET):	$($(STEM)_OBJS)

$(STEM)_clean:	TO_CLEAN := $($(STEM)_OBJS) $(TARGET)

ifeq ($(TGT_TYPE),lib)
$(TARGET):
	$(makelib)
endif

ifeq ($(TGT_TYPE),so)
$(TARGET):
	$(makeso)
endif

ifeq ($(TGT_TYPE),cppso)
$(TARGET):
	$(makecppso)
endif

ifeq ($(TGT_TYPE),exe)
$(TARGET):
	$(makeexe)
endif

ifeq ($(TGT_TYPE),cppexe)
$(TARGET):
	$(makecppexe)
endif

.PHONY: $(STEM)_clean $(STEM)_cleanall

$(STEM)_clean:
	-rm $(TO_CLEAN)

$(STEM)_cleanall: $(STEM)_clean

CLEAN_FILES += $(TARGET) $($(STEM)_OBJS)
DEP_FILES += $($(STEM)_DEPS)
