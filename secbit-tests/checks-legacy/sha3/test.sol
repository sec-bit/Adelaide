pragma solidity 0.5.3;
contract Proof {
    bytes32 public digest;   
    uint public date;

    constructor(string memory data) public {
        digest = sha3(data);
        date = now;
    }

    function check(string calldata data) external view returns (bool) {
        return digest == sha3(data);
    }
}
