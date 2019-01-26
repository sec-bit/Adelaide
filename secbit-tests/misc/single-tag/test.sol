pragma solidity ^0.4.18;
pragma experimental "SMTChecker";

contract Reentrance {

	mapping(address => uint) public balances;

	function donate(address _to) external payable {
		balances[_to] += msg.value;
	}

	function balanceOf(address _who) public view returns (uint balance) {
		return balances[_who];
	}

	function withdraw(uint _amount) public {
		if(balances[msg.sender] >= _amount) {
			(bool x, bytes memory y) = msg.sender.call.value(_amount)("");
			if(x) {
				_amount;
			}
			balances[msg.sender] -= _amount; // Defect
		}
	}

	function withdraw2(uint _amount) public {
		if(balances[msg.sender] >= _amount) {
			balances[msg.sender] -= _amount; // No Defect
			(bool x, bytes memory y) = msg.sender.call.value(_amount)("");
			if(x) {
				_amount;
			}
		}
	}

	function withdraw3(uint _amount) public {
		balances[msg.sender] -= _amount; // No Defect
	}

	function() external payable {}
}
