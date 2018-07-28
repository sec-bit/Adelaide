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
    function transfer(address to, uint tokens) public ;
    function approve(address spender, uint tokens) public ;
    function transferFrom(address from, address to, uint tokens) public;

    event Transfer(address indexed from, address indexed to, uint tokens);
    event Approval(address indexed tokenOwner, address indexed spender, uint tokens);
}

contract NewContract is ERC20Interface {
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	// Ok.
	function balanceOf(address tokenOwner) public view returns (uint balance) {
		return balances[tokenOwner];
	}

	// Bug.
	function transferFrom(address from, address to, uint tokens) public {
		allowed[from][msg.sender] -= tokens;
	}
	// Bug.
	function transfer(address to, uint tokens) public {
		balances[msg.sender] -= tokens;
	}
	// Bug.
	function approve(address spender, uint tokens) public {
	}
}
