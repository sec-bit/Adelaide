pragma solidity ^0.4.18;
pragma experimental "SMTChecker";

contract Reentrance {

	mapping(address => uint) public balances;

	function donate(address _to) public payable {
		balances[_to] += msg.value;
	}

	function balanceOf(address _who) public view returns (uint balance) {
		return balances[_who];
	}

	function withdraw(uint _amount) public {
		if(balances[msg.sender] >= _amount) {
			if(msg.sender.call.value(_amount)()) {
				_amount;
			}
			balances[msg.sender] -= _amount; // Defect
		}
	}

	function withdraw2(uint _amount) public {
		if(balances[msg.sender] >= _amount) {
			balances[msg.sender] -= _amount; // No Defect
			if(msg.sender.call.value(_amount)()) {
				_amount;
			}
		}
	}

	function withdraw3(uint _amount) public {
		balances[msg.sender] -= _amount; // No Defect
	}

	function() public payable {}
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

contract EIP20Interface {
    function totalSupply() public view returns (uint);
    function balanceOf(address tokenOwner) public view returns (uint balance);
    function allowance(address tokenOwner, address spender) public view returns (uint remaining);
    function transfer(address to, uint tokens) public returns (bool success);
    function approve(address spender, uint tokens) public returns (bool success);
    function transferFrom(address from, address to, uint tokens) public returns (bool success);

    event Transfer(address indexed from, address indexed to, uint tokens);
    event Approval(address indexed tokenOwner, address indexed spender, uint tokens);
}

contract TokenInterface {
    function totalSupply() public view returns (uint);
    function balanceOf(address tokenOwner) public view returns (uint balance);
    function allowance(address tokenOwner, address spender) public view returns (uint remaining);
    function transfer(address to, uint tokens) public returns (bool success);
    function approve(address spender, uint tokens) public returns (bool success);
    function transferFrom(address from, address to, uint tokens) public returns (bool success);

    event Transfer(address indexed from, address indexed to, uint tokens);
    event Approval(address indexed tokenOwner, address indexed spender, uint tokens);
}

contract COINIface {
    function totalSupply() public view returns (uint);
    function balanceOf(address tokenOwner) public view returns (uint balance);
    function allowance(address tokenOwner, address spender) public view returns (uint remaining);
    function transfer(address to, uint tokens) public returns (bool success);
    function approve(address spender, uint tokens) public returns (bool success);
    function transferFrom(address from, address to, uint tokens) public returns (bool success);

    event Transfer(address indexed from, address indexed to, uint tokens);
    event Approval(address indexed tokenOwner, address indexed spender, uint tokens);
}

contract NewContract0 is ERC20Interface {
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	function transfer(address to, uint tokens) public returns (bool success) {
		return false;
	}
	// Bug.
	function approve(address spender, uint tokens) public returns (bool success) {
		allowed[spender][msg.sender] = tokens;
		return true;
	}
}

contract NewContract1 is EIP20Interface {
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	function transfer(address to, uint tokens) public returns (bool success) {
		return false;
	}
	// Bug.
	function approve(address spender, uint tokens) public returns (bool success) {
		allowed[spender][msg.sender] = tokens;
		return true;
	}
}

contract Token {
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	function transfer(address to, uint tokens) public returns (bool success) {
		return false;
	}
	// Bug.
	function approve(address spender, uint tokens) public returns (bool success) {
		allowed[spender][msg.sender] = tokens;
		return true;
	}
}

contract myCOIN {
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	function transfer(address to, uint tokens) public returns (bool success) {
		return false;
	}
	// Bug.
	function approve(address spender, uint tokens) public returns (bool success) {
		allowed[spender][msg.sender] = tokens;
		return true;
	}
}

contract NewContract2 is TokenInterface {
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	function transfer(address to, uint tokens) public returns (bool success) {
		return false;
	}
	// Bug.
	function approve(address spender, uint tokens) public returns (bool success) {
		allowed[spender][msg.sender] = tokens;
		return true;
	}
}


contract newContract3 is COINIface {
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	function transfer(address to, uint tokens) public returns (bool success) {
		return false;
	}
	// Bug.
	function approve(address spender, uint tokens) public returns (bool success) {
		allowed[spender][msg.sender] = tokens;
		return true;
	}
}


contract newContract4  {
	mapping(address => uint) balances;
	mapping(address => mapping (address => uint)) allowed;

	function transfer(address to, uint tokens) public returns (bool success) {
		return false;
	}
	function transferFrom(address from, address to, uint tokens) public returns (bool success) {
		return false;
	}
	function approve(address spender, uint tokens) public returns (bool success) {
		allowed[spender][msg.sender] = tokens;
		return true;
	}
}

