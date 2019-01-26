pragma solidity 0.4.18;

contract SafeMath {
  function safeMul(uint a,uint b) internal pure returns (uint) {
    uint c = a * b;
    assert(a == 0 || c / a == b);
    return c;
  }


  function safeDiv(uint a,uint b) internal pure returns (uint) {
    uint c = a / b;
    return c;
  }

  function safeSub(uint a,uint b) internal pure returns (uint) {
    assert(b <= a);
    return a - b;
  }

  function safeAdd(uint a,uint b) internal pure returns (uint) {
    uint c = a + b;
    assert(c>=a && c>=b);
    return c;
  }

}

contract BinomialCoefficient {

    function calculate(uint8 n, uint8 k) external pure returns (uint) {
        return factorial(n) / factorial(k) / factorial(n - k);
    }
    
    function factorial(uint8 n) internal pure returns (uint fact) {
        fact = 1;
        for (uint8 i = n; i > 1; i--) {
            fact *= i;
        }
    }

    function infiniteLoop() public pure {
        for (uint8 i = 100; i >= 0; i--){
		i--;
	}
    }

    function reverted() public pure {
	    uint8 i = 1;
	    i--;
	    i--;
	    revert();
    }

    mapping(address => uint256) balances;
    function batchTransfer(address[] _receivers, uint256 _value) public view returns (bool) {
	    uint cnt = _receivers.length;
	    uint256 amount = uint256(cnt) * _value;
	    require(cnt > 0 && cnt <= 20);
	    require(_value > 0 && balances[msg.sender] >= amount);
	    require(balances[msg.sender] + amount >= balances[msg.sender]); // No Defect.

	    //balances[msg.sender] = balances[msg.sender].sub(amount);
	    for (uint8 i = 0; i < cnt; i++) {
		    //balances[_receivers[i]] = balances[_receivers[i]].add(_value);
		    //Transfer(msg.sender, _receivers[i], _value);
	    }
	    return true;
    }
}
