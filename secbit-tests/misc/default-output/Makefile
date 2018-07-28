include ../../test.mk

TEST_SOL:=test.sol

tests: gen-output cmp-output

# Make sure we don't report SECBIT warning by default.
gen-output:
	@$(SOLC) $(TEST_SOL) 2>&1 > $(output) | true
