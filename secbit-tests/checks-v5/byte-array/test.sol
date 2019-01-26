pragma solidity 0.4.11;
contract C {
    uint other;
    byte[] bad;
    bytes good;
    //Ignore parameter.
    function f(byte[] memory bad, bytes memory good) public pure {
    }
}
