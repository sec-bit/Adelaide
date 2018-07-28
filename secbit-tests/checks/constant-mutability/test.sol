pragma solidity 0.4.11;
contract NewContract{
	mapping(address => uint) public list;
	mapping(uint => address) public listIndex;
	function hasConstant(address _account) public constant returns (bool) {
		return _account == listIndex[list[_account]];
	}
	function hasView(address _account) public view returns (bool) {
		return _account == listIndex[list[_account]];
	}
}
