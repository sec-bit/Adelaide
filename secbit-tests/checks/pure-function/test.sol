pragma solidity 0.4.16;

contract C {
    function f1(uint a, uint b) view public returns (uint) {
        return a;
    }
    function f2(uint a, uint b) public returns (uint) {
        return 2;
    }
}
