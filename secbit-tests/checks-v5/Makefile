SUBDIRS := $(wildcard */.)

all: $(SUBDIRS)
$(SUBDIRS):
	@make --no-print-directory -C $@

.PHONY: all $(SUBDIRS)
