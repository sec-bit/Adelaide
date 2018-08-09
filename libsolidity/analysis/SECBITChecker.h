/*
	The SECBIT Static Analysis Extension to Solidity Compiler

	Copyright (c) SECBIT Labs 2018.

	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#ifdef SECBIT

#include <libsolidity/analysis/TypeChecker.h>
#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTVisitor.h>

namespace dev
{
namespace solidity
{
/**
 * The module that performs SECBIT syntax analysis on the AST:
 *  - [bad-name] naming convension.
 *  - [blockhash] using 'blockhash()'.
 *  - [byte-array] byte array used in contract.
 *  - [delegatecall] use of delegatecall.
 *  - [dirty-padding] msg.data might contain dirty high bits if arg is not of 256bits.
 *  - [fix-version] language version should be fixed.
 *  - [forced-ether] condition based on current balance.
 *  - [hardcode-addr] hard-coded address used in contract.
 *  - [int-div] integer division.
 *  - [no-return] no return stmt in a function with return type.
 *  - [private-modifier] using private modifier.
 *  - [pull-vs-push] send/transfer may fail.
 *  - [redundant-fallback] fallback with a single throw.
 *  - [revert-vs-require] using a conditional revert instead of require.
 *  - [send-vs-transfer] using send instead of transfer.
 *  - [timestamp] using 'now' in a condition.
 *  - [tx-origin] using 'tx.origin' in a condition.
 *
 * The follow ERC20-specific issues are also checked:
 *  - [erc20-no-decimals] ERC20 contract does not have 'decimals'.
 *  - [erc20-no-name] ERC20 contract does not have 'name'.
 *  - [erc20-no-return] ERC20 function is not returning a single boolean.
 *  - [erc20-no-symbol] ERC20 contract does not have 'symbol'.
 *  - [erc20-mintable] mintable ERC20 contract.
 *  - [erc20-return-false] ERC20 function returns false.
 *  - [short-addr] the transfer function is vulnerable to short address attack.
 *  - [transfer-no-revert] the transfer/transferFrom function does not revert.
 *  - [transfer-no-event] the transfer/transferFrom function does not emit Transfer.
 *  - [transferfrom-no-allowed-check] the transferFrom function does not check 'allowed'.
 *  - [approve-no-event] the approve function does not emit Approval.
 *  - [approve-with-balance-verify] the transfer/transferFrom function does revert.
 *
 * The following issues are checked elsewhere:
 *  - [constanpt-mutability] using 'constant'. (parsing/Parser.cpp)
 *  - [implicit-visibility] no visitbility specified. (analysis/StaticAnalyzer.cpp)
 *  - [pure-function] function state mutability can be restricted. (analysis/ViewPureChecker.cpp)
 *  - [view-function] function state mutability can be restricted. (analysis/ViewPureChecker.cpp)
 *  - [reentrance] could subject to reentrance attach. (formal/SMTeChecker.cpp)
 *  - [throw] using 'throw'. (analysis/SyntaxChecker.cpp)
 *  - [type-inference] using inferenced type. (analysis/TypeChecker.cpp)
 *  - [unchecked-math] over/underflow. (formal/SMTeChecker.cpp)
 */

class SECBITChecker: private ASTConstVisitor
{
public:
	/// @param _errorReporter provides the error logging functionality.
	SECBITChecker(
		ErrorReporter& _errorReporter,
		bool _asERC20
	): m_errorReporter(_errorReporter), m_asERC20(_asERC20) {}

	bool checkSyntax(ASTNode const& _astRoot);

private:
	// Visitor callbacks.
	virtual bool visit(ContractDefinition const& _contract) override;
	virtual void endVisit(ContractDefinition const& _contract) override;

	virtual bool visit(FunctionDefinition const& _fn) override;
	virtual void endVisit(FunctionDefinition const& _fn) override;

	virtual bool visit(ModifierDefinition const& _mo) override;
	virtual void endVisit(ModifierDefinition const& _mo) override;

	virtual bool visit(BinaryOperation const& _bin) override;
	virtual void endVisit(BinaryOperation const& _bin) override;

	virtual bool visit(MemberAccess const& _bin) override;
	virtual void endVisit(MemberAccess const& _ma) override;

	virtual bool visit(IfStatement const& _if) override;
	virtual bool visit(Block const& _block) override;
	virtual void endVisit(Assignment const& _assign) override;
	virtual void endVisit(Literal const& _literal) override;
	virtual void endVisit(VariableDeclaration const& _decl) override;
	virtual void endVisit(PragmaDirective const& _pragma) override;
	virtual void endVisit(EventDefinition const& _event) override;
	virtual void endVisit(FunctionCall const& _call) override;
	virtual void endVisit(Identifier const& _id) override;
	virtual void endVisit(Return const& _id) override;
	virtual void endVisit(EmitStatement const& _emit) override;

	// Report ERC20 property-related issues.
	void reportERC20PropertyIssues();

	ErrorReporter& m_errorReporter;

	// Comparison depth. > 0 means we are in a comparision expression.
	int m_comparisonDepth = 0;

	// A parameter which might contain dirty bits.
	const VariableDeclaration *m_dirtyParam = nullptr;

	// Return stmt.
	bool m_hasReturn = false;
	// Return false.
	bool m_returnFalse = false;
	// Has revert/require/assert.
	bool m_hasRevert = false;
	// Poor man's IPA.
	std::set<std::string> m_callablesWithRevert;

	// Inside a if branch.
	bool m_conditional = false;

	// Treat Token/Coin contracts as ERC20 contracts.
	bool m_asERC20 = false;

	// Inside an ERC20 contract.
	bool m_inERC20 = false;
	// Has anti-short-addr assert/require.
	bool m_hasMsgDataCheck = false;
	// Has assert/require like `require(tokens < balances[msg.sender])`.
	bool m_hasSenderBalanceCheck = false;
	// Has assert/require like `require(allowed[x][y] >= ...)`.
	bool m_hasAllowedCheck = false;
	// Emits Approval.
	bool m_emitApproval = false;
	// Emits Transfer.
	bool m_emitTransfer = false;
	// Has `allowed[x][y] - ...`.
	bool m_hasAllowedUnsafeSub = false;
	// Inside a member access to length.
	bool m_inLengthAccess = false;

	// For ERC20 property checks.
	struct ERC20Properties {
		ContractDefinition const* m_defn = nullptr;
	        bool m_hasName = false;
		bool m_hasDecimals = false;
		bool m_hasSymbol = false;
		bool m_mintable = false;
		bool m_isBase = false;
	};
	// A map from ERC20 contract names to their properties.
	std::map<std::string, ERC20Properties> m_ERC20Contracts;
};

}
}
#endif
