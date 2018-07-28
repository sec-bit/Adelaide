pragma solidity 0.4.11;
contract NewContract{
	function donate(address _to) public payable {
		if(!_to.send(42 ether)) {
			revert();
		}
		_to.transfer(42 ether);
		_to.call.value(42 ether);
	}
}
