pragma solidity 0.4.16;

contract C {
    function f1(uint a, uint b) pure public returns (uint) {
        return a;
    }
    function f2(uint a, uint b) pure public returns (uint) {
	    f1(a, b); //defect
	    return f1(a,b); //ok
    }
}
