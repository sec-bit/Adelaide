pragma solidity 0.4.16;

contract C {
	function shouldBePure(uint a, uint b) public returns (uint) {
		return now % a;
	}
	function notPure(address a)  public {
		selfdestruct(a);
		return ;
	}
}
