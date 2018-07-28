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
	mapping(address => uint256) balances;

	// Bug.
	function approve(address spender, uint tokens) public returns (bool success) {
		require(tokens < balances[msg.sender]);
		return true;
	}
}
contract NewContract1 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;
	mapping(address => uint256) balances;
	
	// Bug.
	function approve(address spender, uint tokens) public returns (bool success) {
		require(tokens <= balances[msg.sender]);
		return true;
	}
}
contract NewContract2 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;
	mapping(address => uint256) balances;
	
	// Bug.
	function approve(address spender, uint tokens) public returns (bool success) {
		require(balances[msg.sender] > tokens);
		return true;
	}
}
contract NewContract3 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;
	mapping(address => uint256) balances;
	
	// Bug.
	function approve(address spender, uint tokens) public returns (bool success) {
		require(balances[msg.sender] >= tokens);
		return true;
	}
}

contract NewContract4 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;
	mapping(address => uint256) balances;

	// Ok
	function approve(address spender, uint tokens) public returns (bool success) {
		return false;
	}
	// Ok
	function transfer(address to, uint tokens) public returns (bool success) {
		require(balances[msg.sender] >= tokens);
		require(msg.data.length >= 2*32);
		return false;
	}
}

contract NewContract5 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;
	mapping(address => uint256) balances;

	modifier checkBalance(uint tokens) {
		require(tokens < balances[msg.sender]);
		_;
	}

	// FN, not handling modifiers yet.
	function approve(address spender, uint tokens) public checkBalance(tokens) returns (bool success) {
		return false;
	}
}
