pragma solidity 0.4.11;
contract NewContract{
	function foo1(); // bad
	
	function foo2() public; // good
	function foo3() internal; // good
}
