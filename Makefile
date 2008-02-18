
.PHONY: all dep clean test

all: memlib memtest memdiff splitrec memcat joinrec

include global.mk

Dir := memlib
include memlib/memlib.mk

Dir := memtest
include memtest/memtest.mk

Dir := memdiff
include memdiff/memdiff.mk

Dir := splitrec
include splitrec/splitrec.mk

Dir := memcat
include memcat/memcat.mk

Dir := joinrec
include joinrec/joinrec.mk

include $(DEP_FILES)

dep: $(DEP_FILES)

clean:
	-rm $(CLEAN_FILES)

test: $(TEST_FILES)
	@for tst in $^; do echo Running test $$tst; $$tst; done;
