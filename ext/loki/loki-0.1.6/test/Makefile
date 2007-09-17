SUBTARGETS_ORIG := $(patsubst %/,%,$(dir $(wildcard */Makefile)))
SUBTARGETS_FILTER_OUT := flex_string
SUBTARGETS := $(filter-out $(SUBTARGETS_FILTER_OUT),$(SUBTARGETS_ORIG))
	
SUBTARGETS_CLEAN := $(addsuffix -clean,$(SUBTARGETS))

.PHONY: all clean $(SUBTARGETS) $(SUBTARGETS_CLEAN)
all: $(SUBTARGETS)
clean: $(SUBTARGETS_CLEAN)

$(SUBTARGETS):
	$(MAKE) -C $@

$(SUBTARGETS_CLEAN):
	$(MAKE) -C $(@:-clean=) clean
