pragma solidity 0.4.11;
contract Holder {
    uint public holdUntil;   
    address public holder;
    
    function withdraw() external {
	require(now > holdUntil);
        suicide(holder);
    }
}
