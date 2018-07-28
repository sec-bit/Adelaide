pragma solidity 0.4.18;

contract ForceEther {

  bool public youWin = false;

  function onlyNonZeroBalance() public {
      require(this.balance > 0); 
      youWin = true;
  }
  // throw if any ether is received
  function() payable public {
    revert();
  }
         
}
