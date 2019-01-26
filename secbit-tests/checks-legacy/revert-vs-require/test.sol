pragma solidity 0.4.11;

contract Holder {
    uint public holdUntil;
    address public holder;

    function withdraw (uint a, uint b) external {
        if (now < holdUntil){
            revert();
        }
        holder.transfer(this.balance);
    }
}
