pragma solidity ^0.4.18;
pragma experimental "SMTChecker";

contract Reentrance {

	mapping(address => uint) public balances;

	function donate(address _to) public payable {
		balances[_to] += msg.value;
	}

	function balanceOf(address _who) public view returns (uint balance) {
		return balances[_who];
	}

	function withdraw(uint _amount) public {
		if(balances[msg.sender] >= _amount) {
			if(msg.sender.call.value(_amount)()) {
				_amount;
			}
			balances[msg.sender] -= _amount; // Defect
		}
	}

	function withdraw2(uint _amount) public {
		if(balances[msg.sender] >= _amount) {
			balances[msg.sender] -= _amount; // No Defect
			if(msg.sender.call.value(_amount)()) {
				_amount;
			}
		}
	}

	function withdraw3(uint _amount) public {
		balances[msg.sender] -= _amount; // No Defect
	}

	function() public payable {}
}
