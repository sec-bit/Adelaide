pragma solidity 0.4.11;
contract NewContract{
	function f() public pure {
		var j = 5555;
		for (var i = 0; i < j; i++) {
			i++;/* ... */
		}
	}
}
