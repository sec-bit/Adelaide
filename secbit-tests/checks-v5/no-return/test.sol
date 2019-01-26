pragma solidity 0.4.11;

contract NewContract{
	uint256 _minimumBuy;
	function f1(uint256 newMinimumBuy) public returns (bool){
		_minimumBuy = newMinimumBuy;
	}
	function f2(uint256 newMinimumBuy) public view returns (bool){
		return _minimumBuy == newMinimumBuy;
	}
	function f3(uint256 newMinimumBuy) public view returns (bool b){
	        b = _minimumBuy == newMinimumBuy;
	}
}
