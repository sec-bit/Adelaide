pragma solidity 0.4.11;
contract C {
    uint other;
    byte[] bad;
    bytes good;
    //Ignore parameter.
    function f(byte[] bad, bytes good) public pure {
    }
}
