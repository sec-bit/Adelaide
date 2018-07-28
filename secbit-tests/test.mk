SECBIT_TEST_ROOT:=$(dir $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
SOL_ROOT:=$(SECBIT_TEST_ROOT)/..
SOL_BUILD:=$(SOL_ROOT)/build
SOLC:=$(SOL_BUILD)/solc/solc
PAR:=-j$(shell grep -c ^processor /proc/cpuinfo)

tests: $(SOLC)

$(SOLC):
	@mkdir -p $(SOL_BUILD)
	cd $(SOL_BUILD); cmake .. && make solc $(PAR)

output:=$(CURDIR)/output
expected:=$(CURDIR)/expected
SOLC_SECBIT:=$(SOLC) --secbit-warnings output -o . --overwrite

cmp-output: gen-output
	@diff $(output) $(expected) && echo "Passed" $(CURDIR)

update: gen-output
	@mv $(output) $(expected) && echo "Passed" $(CURDIR)

.PHONY: gen-output cmp-putput update tests
