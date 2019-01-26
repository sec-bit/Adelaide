pragma solidity 0.4.11;

contract Holder {
    function f1 (uint8 a) pure external {
	    require(keccak256(msg.data) == 0);
    }
    function f2 (uint256 a) pure external {
	    require(keccak256(msg.data) == 0);
    }
}
