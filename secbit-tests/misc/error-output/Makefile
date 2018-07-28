include ../../test.mk

TEST_SOL:=test.sol

tests: gen-output cmp-output

# Make sure we still report error when --secbit-warnings.
gen-output:
	@$(SOLC_SECBIT) $(TEST_SOL) 2> $(output) | true
