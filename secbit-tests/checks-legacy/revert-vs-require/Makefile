include ../../test.mk

TEST_SOL:=test.sol

tests: gen-output cmp-output

gen-output:
	@$(SOLC_SECBIT) $(TEST_SOL)
