pragma solidity 0.4.11;
contract NewContract{
	function foo() public; // good
	event LogFoo(); // good
	event badlyNamedEvent();
	function BadlyNamedFunction() external pure {}
}
