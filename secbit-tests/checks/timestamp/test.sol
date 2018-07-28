pragma solidity 0.4.11;
contract NewContract{
	function oddOrEven(bool yourGuess) external payable returns (bool) {
		if (yourGuess == (now % 2 == 0)) {
			msg.sender.transfer(msg.value);
		}
		return true;
	}
}
