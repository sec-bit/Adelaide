pragma solidity 0.4.11;
contract Proof {
    bytes32 public digest;   
    uint public date;

    function Proof(string data) public {
        digest = sha3(data);
        date = now;
    }

    function check(string data) external view returns (bool) {
        return digest == sha3(data);
    }
}
