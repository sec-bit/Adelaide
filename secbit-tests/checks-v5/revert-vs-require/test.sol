pragma solidity 0.4.11;

contract Holder {
    uint public holdUntil;

    function withdraw (uint a, uint b) external {
        address payable holder;
        if (now < holdUntil){
            revert();
        }
        holder.transfer(address(this).balance);
    }
}
