include ../../test.mk

TEST_SOL:=test.sol

tests: gen-output cmp-output

gen-output:
	@$(SOLC_SECBIT) --erc20 --secbit-tag erc20-mintable $(TEST_SOL)
