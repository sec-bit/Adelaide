pragma solidity 0.4.18;

library SafeMath {
    function add(uint a, uint b) internal pure returns (uint c) {
        c = a + b;
        require(c >= a);
    }
    function sub(uint a, uint b) internal pure returns (uint c) {
        require(b <= a);
        c = a - b;
    }
    function mul(uint a, uint b) internal pure returns (uint c) {
        c = a * b;
        require(a == 0 || c / a == b);
    }
    function div(uint a, uint b) internal pure returns (uint c) {
        require(b > 0);
        c = a / b;
    }
}

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
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	// Bug.
	function transferFrom(address from, address to, uint tokens) public returns (bool success) {
		require(msg.data.length >= 2*32);
		allowed[from][msg.sender] -= tokens;
		Transfer(from, to, tokens);
		return true;
	}
}
contract NewContract0 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	// Bug.
	function transferFrom(address from, address to, uint tokens) public returns (bool success) {
		require(msg.data.length >= 2*32);
		allowed[from][msg.sender] = allowed[from][msg.sender] - tokens;
		Transfer(from, to, tokens);
		return true;
	}
}
contract NewContract1 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	// Ok.
	function transferFrom(address from, address to, uint tokens) public returns (bool success) {
		require(msg.data.length >= 2*32);
		Transfer(from, to, tokens);
		return true;
	}
}
contract NewContract2 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;
	mapping(address => uint256) balances;
	mapping(address => mapping (address => uint256)) allowed;

	// Ok.
	function transferFrom(address from, address to, uint tokens) public returns (bool success) {
		require(msg.data.length >= 2*32);
		require(allowed[from][msg.sender] >= tokens);
		allowed[from][msg.sender] -= tokens;
		Transfer(from, to, tokens);
		return true;
	}
}

contract NewContract3 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;
	using SafeMath for uint;
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	// Ok.
	function transferFrom(address from, address to, uint tokens) public returns (bool success) {
		require(msg.data.length >= 2*32);
		allowed[from][msg.sender] = allowed[from][msg.sender].sub(tokens);
		Transfer(from, to, tokens);
		return true;
	}
}

contract NewContract4 is ERC20Interface {
	uint8 public decimals;
	string public name;
	string public symbol;
	using SafeMath for uint;
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	// Ok.
	function transferFrom(address from, address to, uint tokens) public returns (bool success) {
		require(msg.data.length >= 2*32);
		uint x = allowed[from][msg.sender];
		require(x >= tokens);
		allowed[from][msg.sender] -= tokens;
		Transfer(from, to, tokens);
		return true;
	}
}
