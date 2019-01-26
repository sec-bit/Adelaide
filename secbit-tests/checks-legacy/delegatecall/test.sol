pragma solidity 0.4.11;

// Credits to OpenZeppelin for this contract taken from the Ethernaut CTF
// https://ethernaut.zeppelin.solutions/level/0x68756ad5e1039e4f3b895cfaa16a3a79a5a73c59
contract Delegate {

  address public owner;

  function Delegate(address _owner) public {
    owner = _owner;
  }

  function pwn() public {
    owner = msg.sender;
  }
}

library M {
	function f(uint256 a, uint256 b) returns (uint256) {
		return a;
	}
}

contract Delegation {

  address public owner;
  Delegate delegate;
  using M for uint;

  function Delegation(address _delegateAddress) public {
    delegate = Delegate(_delegateAddress);
    owner = msg.sender;
  }

  // an attacker can call Delegate.pwn() in the context of Delegation
  // this means that pwn() will modify the state of **Delegation** and not Delegate
  // the result is that the attacker takes unauthorized ownership of the contract
  function() public {
    if(delegate.delegatecall(msg.data)) {
      this;
    }
    uint x = 1;
    // This is compiled to a Kind::Delegatecall, but we choose to not report.
    x.f(2);
  }
}
