pragma solidity 0.4.11;
interface SomeInterface {
    function foo() public;
    function bar() public;
}

contract NotBar is SomeInterface {
    event Foo(address callee);
    function foo() public {
        Foo(msg.sender);   
    }
    function bar() public {
        throw;   
    }
}
