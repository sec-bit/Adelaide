pragma solidity 0.4.11;

contract NewContract{
	function f1(uint256 lastBlockNumberC) public view {
		block.blockhash(lastBlockNumberC);
		blockhash(lastBlockNumberC);
	}
}
