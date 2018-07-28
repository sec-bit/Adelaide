pragma solidity 0.4.18;

contract ERC20Interface {
    function totalSupply() public view returns (uint);
    function balanceOf(address tokenOwner) public view returns (uint balance);
    function allowance(address tokenOwner, address spender) public view returns (uint remaining);
    function transfer(address to, uint tokens) public returns (bool success);
    function approve(address spender, uint tokens) public returns (bool success);
    function transferFrom(address from, address to, uint tokens) public returns (bool success);

    event Transfer(address indexed from, address indexed to, uint tokens);
    event Approval(address indexed tokenOwner, address indexed spender, uint tokens);
}

contract NewContract is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;

	modifier onlyPayloadSize(uint size) {
		require(msg.data.length >= size + 4);
		_;
	}
	function transfer(address to, uint tokens) public onlyPayloadSize(2 * 32) returns (bool success) {
		return false;
	}
	function transferFrom(address to, uint tokens) public pure returns (bool success) {
		require(tokens > 0);
		return false;
	}
}

contract NewContract2 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;

	modifier onlyPayloadSize(uint size) {
		require(msg.data.length >= size + 4);
		_;
	}
	function transfer(address to, uint tokens) public returns (bool success) {
		require(msg.data.length >= 2*32);
		return false;
	}
}
