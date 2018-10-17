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
#ifdef SECBIT

#include <libsolidity/analysis/SECBITChecker.h>
#include <memory>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ExperimentalFeatures.h>
#include <libsolidity/analysis/SemVerHandler.h>
#include <libsolidity/interface/ErrorReporter.h>
#include <libsolidity/interface/Version.h>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

bool SECBITChecker::checkSyntax(ASTNode const& _astRoot)
{
	_astRoot.accept(*this);
	reportERC20PropertyIssues();
	return Error::containsOnlyWarnings(m_errorReporter.errors());
}

void SECBITChecker::reportERC20PropertyIssues()
{
	for(auto const& p: m_ERC20Contracts) {
		if(p.second.m_mintable) {
			m_errorReporter.secbitWarning(
				p.second.m_defn->location(),
				"erc20-mintable",
				"This ERC20 contract could be mintable.");
		}
		// Do not report on base contracts.
		if(p.second.m_isBase) {
			continue;
		}
		if(!p.second.m_hasDecimals) {
			m_errorReporter.secbitWarning(
				p.second.m_defn->location(),
				"erc20-no-decimals",
				"This ERC20 contract does not have a 'decimals' state "
				"variable or a 'decimals()' function.");
		}
		if(!p.second.m_hasName) {
			m_errorReporter.secbitWarning(
			        p.second.m_defn->location(),
				"erc20-no-name",
				"This ERC20 contract does not have a 'name' state "
				"variable or a 'name()' function.");
		}
		if(!p.second.m_hasSymbol) {
			m_errorReporter.secbitWarning(
				p.second.m_defn->location(),
				"erc20-no-symbol",
				"This ERC20 contract does not have a 'symbol' state "
				"variable or a 'symbol()' function.");
		}
	}
}

static bool isERC20Name(string const& _name, bool _asERC20)
{
	string const lower{boost::algorithm::to_lower_copy(_name)};
	auto contains = [&](char const* _id) {
		return lower.find(_id) != string::npos;
	};
	return (contains("erc20") ||
		contains("eip20") ||
		// If `--erc20`, consider any name containing token/coin
		// as an ERC20 name.
		(_asERC20 &&
		 (contains("token") || contains("coin"))));
}

// Logic for detecting possible ERC20 contracts.
// If `--erc20` is not sepcified:
//     the name of the contract or any base contract contains
//     erc20/eip20 (case insensitive).
// If `--erc20` is specified:
//     the name of the contract or any base contract contains
//     erc20/eip20/token/coin (case insensitive), or
//     a contract with `transfer`, `transferFrom`, and `approve`.
static bool isERC20Contract(ContractDefinition const& _contract, bool _asERC20)
{
	// Not for libraries.
	if(_contract.isLibrary()) {
		return false;
	}

	// Not for partially implemented contracts (interfaces).
	for(auto const* fn : _contract.definedFunctions()) {
		if(fn && !fn->isImplemented()) {
			return false;
		}
	}

	// Check base names.
	for(auto &p : _contract.baseContracts()) {
		const auto &path = p->name().namePath();
		if(isERC20Name(path.back(), _asERC20)) {
			return true;
		}
	}

	// Check contract name.
	if(isERC20Name(_contract.name(), _asERC20)) {
		return true;
	}

	// Finally, relax to allow contracts with the three APIs.
	if(_asERC20) {
		bool hasTransfer = false;
		bool hasTransferFrom = false;
		bool hasApprove = false;
		for(auto const* fn : _contract.definedFunctions()) {
			if(fn) {
				if(fn->name() == "transfer") {
					hasTransfer = true;
				} else if(fn->name() == "transferFrom") {
					hasTransferFrom = true;
				} else if(fn->name() == "approve") {
					hasApprove = true;
				}
			}
		}
		return hasTransfer && hasTransferFrom && hasApprove;
	}
	return false;
}

bool SECBITChecker::visit(ContractDefinition const& _contract)
{
	m_inERC20 = isERC20Contract(_contract, m_asERC20);

	// Skip "SafeMath". Consider all member function having revert.
	if(_contract.name() == "SafeMath") {
		for(const auto *p : _contract.definedFunctions()) {
			m_callablesWithRevert.insert(p->name());
		}
		return false;
	}

	if(m_inERC20) {
		set<string> names;
		bool mintable = false;
		for(auto const* v: _contract.stateVariables()) {
			names.insert(v->name());
		}
		for(auto const* f: _contract.definedFunctions()) {
			names.insert(f->name());
			static boost::regex mintName("^_?mint.*$");
			if(boost::regex_match(f->name(), mintName)) {
				mintable = true;
			}
		}
		string const& name = _contract.name();
		m_ERC20Contracts[name].m_defn = &_contract;
		m_ERC20Contracts[name].m_hasName = names.count("name") != 0;
		m_ERC20Contracts[name].m_hasDecimals = names.count("decimals") != 0;
		m_ERC20Contracts[name].m_hasSymbol = names.count("symbol") != 0;
		m_ERC20Contracts[name].m_mintable = mintable;

		for(auto &p : _contract.baseContracts()) {
			const auto &baseName = p->name().namePath().back();
			if(m_ERC20Contracts.count(baseName) != 0) {
				m_ERC20Contracts[name].m_hasName |=
					m_ERC20Contracts[baseName].m_hasName;
				m_ERC20Contracts[name].m_hasDecimals |=
					m_ERC20Contracts[baseName].m_hasDecimals;
				m_ERC20Contracts[name].m_hasSymbol |=
					m_ERC20Contracts[baseName].m_hasSymbol;
				m_ERC20Contracts[name].m_mintable |=
					m_ERC20Contracts[baseName].m_mintable;
				m_ERC20Contracts[baseName].m_isBase = true;
			}
		}
	}

	return true;
}

void SECBITChecker::endVisit(ContractDefinition const& _contract)
{
	(void) _contract;
	m_inERC20 = false;
}

void SECBITChecker::endVisit(Literal const& _literal)
{
	TypePointer type = _literal.annotation().type;

	if(!type) {
		return;
	}

	if(IntegerType const* it = asC<IntegerType>(type.get())) {
		if(it->isAddress()) {
			m_errorReporter.secbitWarning(
				_literal.location(),
				"hardcode-addr",
				"Hard-coded address should be checked.");
		}
	}
}

void SECBITChecker::endVisit(VariableDeclaration const& _decl)
{
	if (_decl.visibility() == Declaration::Visibility::Private) {
		m_errorReporter.secbitWarning(
			_decl.location(),
			"private-modifier",
			"Private object is still publicly visible on the chain.");
	}

	TypePointer type = _decl.annotation().type;

	if(!type || _decl.isCallableParameter()) {
		return;
	}

	if(auto const* arr = asC<ArrayType>(type.get())) {
		if(!arr->isByteArray() && !arr->isString()){
			if(auto const* elem = asC<FixedBytesType>(arr->baseType().get())) {
				if(elem->numBytes() == 1) {
					m_errorReporter.secbitWarning(
						_decl.location(),
						"byte-array",
						"Consider using 'bytes' to reduce gas consumption "
						"in the declaration of '" + _decl.name() + "'.");
				}
			}
		}
	}
}

void SECBITChecker::endVisit(PragmaDirective const& _pragma)
{
	if (_pragma.literals()[0] == "solidity" && _pragma.literals()[1][0] == '^') {
		m_errorReporter.secbitWarning(
			_pragma.location(),
			"fix-version",
			"Consider using exact language version instead.");
	}
}

// Match if _ma is a member access `_obj[_member]`.
static bool isMemberAccess(MemberAccess const& _ma, string const& _obj, string const& _member)
{
	if(_ma.memberName() != _member) {
		return false;
	}
	if(auto const* id = asC<Identifier>(&_ma.expression())) {
		return id->name() == _obj;
	}
	return false;
}

bool SECBITChecker::visit(MemberAccess const& _ma)
{
	if(_ma.memberName() == "length") {
		m_inLengthAccess = true;
	}
	return true;
}

void SECBITChecker::endVisit(MemberAccess const& _ma)
{
	// Avoid `msg.data.length`.
	if(!m_inLengthAccess && m_dirtyParam && isMemberAccess(_ma, "msg", "data")) {
		m_errorReporter.secbitWarning(
			_ma.location(),
			"dirty-padding",
			"'msg.data' could contain dirty padding because parameter '" +
			m_dirtyParam->name() + "' has a type of " +
			to_string(m_dirtyParam->type()->calldataEncodedSize(/*_padded*/false)*8) + " bits.");
	}

	if(m_comparisonDepth > 0) {
		if(isMemberAccess(_ma, "tx", "origin")) {
			m_errorReporter.secbitWarning(
				_ma.location(),
				"tx-origin",
				"Comparing 'tx.origin' might not have the intended result.");
		}
		if(isMemberAccess(_ma, "this", "balance")) {
			m_errorReporter.secbitWarning(
				_ma.location(),
				"forced-ether",
				"A condition on 'this.balance' could be circumvented "
				"because ether could be forced into a contract via "
				"'selfdestruct'.");
		}
	}

	if(_ma.memberName() == "length") {
		m_inLengthAccess = false;
	}
}


void SECBITChecker::endVisit(Identifier const& _id)
{
        if(m_comparisonDepth <= 0) {
		return;
	}

        if(_id.name() == "now") {
		m_errorReporter.secbitWarning(
			_id.location(),
			"timestamp",
			"Avoid using 'now' in control expression, "
			"since it could be controled by miners.");
	}
}

// Matches `balances[msg.sender]`.
static bool isSenderBalance(Expression const& _expr)
{
	auto const* ia = asC<IndexAccess>(&_expr);
	if(!ia) {
		return false;
	}
        auto const* balances = asC<Identifier>(&ia->baseExpression());
	if(!balances || balances->name() != "balances") {
		return false;
	}
	auto const* ma = asC<MemberAccess>(ia->indexExpression());
	if(!ma) {
		return false;
	}
	return isMemberAccess(*ma, "msg", "sender");
}

// Matches a check of `balances[msg.sender]`
static bool isSenderBalanceCheck(BinaryOperation const* /*non-null*/_bin)
{
	// < or <= balances[msg.sender]
	return (((_bin->getOperator() == Token::LessThan) ||
		 (_bin->getOperator() == Token::LessThanOrEqual)) &&
		isSenderBalance(_bin->rightExpression())) ||
	       (((_bin->getOperator() == Token::GreaterThan) ||
		 (_bin->getOperator() == Token::GreaterThanOrEqual)) &&
		isSenderBalance(_bin->leftExpression()));
}

// Matches `allowed[x][y]`
static bool isAllowed(Expression const& _expr)
{
	auto const* ia = asC<IndexAccess>(&_expr);
	if(!ia) {
		return false;
	}
	auto const* iaInner = asC<IndexAccess>(&ia->baseExpression());
	if(!iaInner) {
		return false;
	}
        auto const* id = asC<Identifier>(&iaInner->baseExpression());
	if(!id) {
		return false;
	}
	return id->name() == "allowed";
}

// Matches a check of `allowed[x][y]`
static bool isAllowedCheck(BinaryOperation const* /*non-null*/_bin)
{
	// < or <= allowed[x][y]
	return (((_bin->getOperator() == Token::LessThan) ||
		 (_bin->getOperator() == Token::LessThanOrEqual)) &&
		isAllowed(_bin->rightExpression())) ||
	       (((_bin->getOperator() == Token::GreaterThan) ||
		 (_bin->getOperator() == Token::GreaterThanOrEqual)) &&
		isAllowed(_bin->leftExpression()));
}

void SECBITChecker::endVisit(Assignment const& _assign)
{
	if(_assign.assignmentOperator() == Token::AssignSub &&
	   isAllowed(_assign.leftHandSide())) {
		m_hasAllowedUnsafeSub = true;
	}
}

bool SECBITChecker::visit(BinaryOperation const& _bin)
{
	if(Token::isCompareOp(_bin.getOperator())) {
		m_comparisonDepth ++;

		// allowed is compared.
		if(isAllowed(_bin.leftExpression()) ||
		   isAllowed(_bin.rightExpression())) {
			   m_hasAllowedCheck = true;
		}
	}
	return true;
}

void SECBITChecker::endVisit(BinaryOperation const& _bin)
{
	TypePointer const& leftType = _bin.leftExpression().annotation().type;
	TypePointer const& rightType = _bin.rightExpression().annotation().type;

	if(_bin.getOperator() == Token::Div) {
		bool warn = is<IntegerType>(leftType.get()) && is<IntegerType>(rightType.get());
		if(!warn) {
			if(auto const* lt = asC<RationalNumberType>(leftType.get())) {
				if(auto const* rt = asC<RationalNumberType>(rightType.get())) {
					warn = !lt->isFractional() && !rt->isFractional();
				}
			}
		}
		if(warn) {
			m_errorReporter.secbitWarning(
				_bin.location(),
				"int-div",
				"Integer division should be used with caution.");
		}
	}
	if(Token::isCompareOp(_bin.getOperator())) {
		m_comparisonDepth --;
	}

	if(_bin.getOperator() == Token::Sub &&
	   isAllowed(_bin.leftExpression())) {
		m_hasAllowedUnsafeSub = true;
	}
}

void SECBITChecker::endVisit(Return const& _r)
{
	if(const auto *lit = asC<Literal>(_r.expression())) {
		if(lit->token() == Token::FalseLiteral) {
			m_returnFalse = true;
		}
	}
	m_hasReturn = true;
}

void SECBITChecker::endVisit(EmitStatement const& _emit)
{
	auto const& call = _emit.eventCall();
	if(auto const* id = asC<Identifier>(&call.expression())) {
		if(id->name() == "Approval") {
			m_emitApproval = true;
		} else if(id->name() == "Transfer") {
			m_emitTransfer = true;
		}
	}
}

void SECBITChecker::endVisit(EventDefinition const& _event)
{
        if (islower(_event.name()[0])) {
		m_errorReporter.secbitWarning(
			_event.location(),
			"bad-name",
			"Event name '" + _event.name() +
			"' should start with an upper-case character.");
	}
}

bool SECBITChecker::visit(ModifierDefinition const& _mo)
{
	(void)_mo;
	m_hasRevert = false;
	return true;
}

void SECBITChecker::endVisit(ModifierDefinition const& _mo)
{
	(void)_mo;
	if(m_hasRevert) {
		m_callablesWithRevert.insert(_mo.name());
		m_hasRevert = false;
	}
}

bool SECBITChecker::visit(FunctionDefinition const& _fn)
{
	// Set revert flag.
	m_hasRevert = false;
	// If modifiers have revert, then we are fine.
	for(auto& p : _fn.modifiers()) {
		if(m_callablesWithRevert.count(p->name()->name())) {
			m_hasRevert = true;
		}
	}

	// Set m_dirtyParam if a parameter is less than 256bits.
	for(auto& p : _fn.parameterList().parameters()) {
		if(!p->type()->isDynamicallyEncoded() &&
		   p->type()->calldataEncodedSize(/*_padded*/false) < 32) {
			m_dirtyParam = p.get();
			break;
		}
	}

	// Clear flags.
	m_hasAllowedCheck = false;
	m_hasAllowedUnsafeSub = false;
	m_hasReturn = false;
	m_returnFalse = false;
	m_hasSenderBalanceCheck = false;
	m_emitApproval = false;
	m_emitTransfer = false;

	return true;
}

void SECBITChecker::endVisit(FunctionDefinition const& _fn)
{
	// ERC20 functions.
	if(m_inERC20) {
		bool retBool =
			_fn.returnParameters().size() == 1 &&
			_fn.returnParameters().front()->type() &&
			_fn.returnParameters().front()->type()->category() == Type::Category::Bool;

		if(m_returnFalse) {
			m_errorReporter.secbitWarning(
				_fn.location(),
				"erc20-return-false",
				"This ERC20 function returns false, "
				"which may not be correctly handled by the caller.");
			m_returnFalse = false;
		}

		if(_fn.name() == "transfer" || _fn.name() == "transferFrom") {
			bool hasModifier = false;
			for(auto& p : _fn.modifiers()) {
				if(p->name()->name() == "onlyPayloadSize") {
					hasModifier = true;
					break;
				}
			}
			if(!hasModifier && !m_hasMsgDataCheck) {
				m_errorReporter.secbitWarning(
					_fn.location(),
					"short-addr",
					"Missing check on 'msg.data.length' "
					"could lead to short-address attack "
					"in this ERC20 transfer function.");
			}
			if(_fn.name() == "transferFrom" &&
			   !m_hasAllowedCheck &&
			   m_hasAllowedUnsafeSub) {
				m_errorReporter.secbitWarning(
					_fn.location(),
					"transferfrom-no-allowed-check",
					"No requirement on 'allowed' could "
					"make this function incompatible with "
					"an ERC20 'transferFrom'.");
			}
			if(!m_hasRevert) {
				m_errorReporter.secbitWarning(
					_fn.location(),
					"transfer-no-revert",
					"No revert inside a transfer function, "
					"which is incompatible with an ERC20.");
			}
			if(!m_emitTransfer) {
				m_errorReporter.secbitWarning(
					_fn.location(),
					"transfer-no-event",
					"This transfer function does not emit 'Transfer' event, "
					"which is incompatible with an ERC20.");
			}
			if(!retBool) {
				m_errorReporter.secbitWarning(
					_fn.location(),
					"erc20-no-return",
					"This ERC20 transfer function is not returning a single bool, "
					"which is incompatible with an ERC20.");
			}
		} else if(_fn.name() == "approve") {
			if(m_hasSenderBalanceCheck) {
				m_errorReporter.secbitWarning(
					_fn.location(),
					"approve-with-balance-verify",
					"Adding requirement on the balance of 'msg.sender' could "
					"make this function incompatible with an ERC20 'approve'.");
			}
			if(!m_emitApproval) {
				m_errorReporter.secbitWarning(
					_fn.location(),
					"approve-no-event",
					"This approve function does not emit 'Approve' event, "
					"which is incompatible with an ERC20.");
			}
			if(!retBool) {
				m_errorReporter.secbitWarning(
					_fn.location(),
					"erc20-no-return",
					"This ERC20 approve function is not returning a single bool, "
					"which is incompatible with an ERC20.");
			}
		}
	}
	m_hasMsgDataCheck = false;
	m_hasSenderBalanceCheck = false;
	m_hasAllowedCheck = false;
	m_hasAllowedUnsafeSub = false;
	m_emitApproval = false;
	m_emitTransfer = false;

	if(!m_hasReturn && _fn.isImplemented()){
		bool hasImplicit = false;
		for(const auto& p : _fn.returnParameters()) {
			if(p->name() == "") {
				hasImplicit = true;
				break;
			}
		}
		if(hasImplicit) {
			m_errorReporter.secbitWarning(
				_fn.location(),
				"no-return",
				"Missing return statement in a function with implicit return parameter.");
		}
	}

	// Reset the pointer.
	m_dirtyParam = nullptr;

	if(isupper(_fn.name()[0]) &&
	    // Do not warn on constructor.
	    !_fn.isConstructor()) {
		m_errorReporter.secbitWarning(
			_fn.location(),
			"bad-name",
			"Function name '"+ _fn.name() +
			"' should start with a lower-case character.");
	}
	if(_fn.isFallback()) {
		if(Block const *body = asC<Block>(&_fn.body())){
			const auto& statements = body->statements();
			if(statements.size() == 1 &&
			   is<Throw>(statements[0].get())) {
				m_errorReporter.secbitWarning(
					_fn.location(),
					"redundant-fallback",
					"Fallback function with a single 'Throw' is no longer "
					"needed after version 0.4.0. Consider removing this function.");
			}
		}
	}

	if(m_hasRevert) {
		m_callablesWithRevert.insert(_fn.name());
		m_hasRevert = false;
	}
}

// Matches address `_expr._member`.
static bool isAddressAccess(Expression const& _expr, string const& _member)
{
	auto const* ma = asC<MemberAccess>(&_expr);
	if(!ma) {
		return false;
	}

	auto const* i = asC<IntegerType>(ma->expression().annotation().type.get());
	if(i && i->isAddress() && ma->memberName() == _member) {
		return true;
	}

	return false;
}

void SECBITChecker::endVisit(FunctionCall const& _call)
{
	auto const* funType = asC<FunctionType>(_call.expression().annotation().type.get());
	if(!funType) {
		return;
	}

	if(auto const* id = asC<Identifier>(&_call.expression())) {
		if(id->name() == "Approval") {
			m_emitApproval = true;
		} else if(id->name() == "Transfer") {
			m_emitTransfer = true;
		}
	}

	if((funType->kind() == FunctionType::Kind::DelegateCall ||
	    funType->kind() == FunctionType::Kind::BareDelegateCall)) {
		if(auto const* ma = asC<MemberAccess>(&_call.expression())) {
			if(ma->memberName() == "delegatecall") {
				m_errorReporter.secbitWarning(
					_call.location(),
					"delegatecall",
					"'delegatecall' should be used with caution because it uses the current context.");
			}
		}
	} else if(funType->kind() == FunctionType::Kind::Send) {
		m_errorReporter.secbitWarning(
			_call.location(),
			"send-vs-transfer",
			"Calling 'send()' requires manual exception handling. "
			"Consider using 'transfer()' instead.");
	} else if(funType->kind() == FunctionType::Kind::Revert) {
		m_hasRevert = true;
		if(m_conditional) {
			m_errorReporter.secbitWarning(
			_call.location(),
			"revert-vs-require",
			"Calling 'require()' instead of a conditional 'revert()' could improve readability.");
		}
	} else if(funType->kind() == FunctionType::Kind::Require) {
		m_hasRevert = true;
		if(auto const* bin = asC<BinaryOperation>(_call.arguments()[0].get())) {
			// require(msg.data.length >= ..
			if(auto const* ma = asC<MemberAccess>(&bin->leftExpression())) {
				if(auto const* ma2 = asC<MemberAccess>(&ma->expression())) {
					if(isMemberAccess(*ma2, "msg", "data") &&
					   ma->memberName() == "length") {
						m_hasMsgDataCheck = true;
					}
				}
			}
			// require(tokens < balances[msg.sender]
			if(isSenderBalanceCheck(bin)) {
				m_hasSenderBalanceCheck = true;
			}
			// require(allowed[x][y] > ...)
			if(isAllowedCheck(bin)) {
				m_hasAllowedCheck = true;
			}
		}
	} else if(funType->kind() == FunctionType::Kind::Assert) {
		m_hasRevert = true;
	} else if(funType->kind() == FunctionType::Kind::BlockHash) {
		m_errorReporter.secbitWarning(
			_call.location(),
			"blockhash",
			"Return value of 'blockhash()' could be controled by miners.");
	}

	if(auto const* id = asC<Identifier>(&_call.expression())) {
		if(m_callablesWithRevert.count(id->name())) {
			m_hasRevert = true;
		}
	}

	// xxx.call.value()
	if(MemberAccess const *ma = asC<MemberAccess>(&_call.expression())) {
		if(m_callablesWithRevert.count(ma->memberName())) {
			m_hasRevert = true;
		}
		if(ma->memberName() == "value" && isAddressAccess(ma->expression(), "call")) {
			m_errorReporter.secbitWarning(
				_call.location(),
				"send-vs-transfer",
				"Calling 'call.value()' does not have 2300 gas limit, "
				"which could lead to reentrance attack. "
				"Consider using 'transfer()' instead.");
		}
	}

}

bool SECBITChecker::visit(IfStatement const& _if)
{
	_if.condition().accept(*this);

	// Mark true/false branch as conditional.
	m_conditional = true;
	_if.trueStatement().accept(*this);
	if(_if.falseStatement()) {
		_if.falseStatement()->accept(*this);
	}
	m_conditional = false;
	return false;
}

bool SECBITChecker::visit(Block const& _block)
{
	const Statement *send = nullptr;
	for(auto& stmt : _block.statements()) {
		if(send) {
			// We have code following a send().
			m_errorReporter.secbitWarning(
				send->location(),
				"pull-vs-push",
				"This call may result in exception and make the following logic dead code. "
				"Consider using pull for ether transfer instead.");
			send = nullptr;
		} else if(auto const* e = asC<ExpressionStatement>(stmt.get())) {
			// require(xxx.send()); require(xxx.transfer());
			if(auto const* call = asC<FunctionCall>(&e->expression())) {
				if(auto const* require = asC<Identifier>(&call->expression())) {
					if(require->name() == "require" && call->arguments().size() == 1) {
						if(auto const* innerCall = asC<FunctionCall>(call->arguments()[0].get())) {
							if(isAddressAccess(innerCall->expression(), "send") ||
							   isAddressAccess(innerCall->expression(), "transfer")) {
								send = stmt.get();
								continue;
							}
						}
					}
				}
			}
		}
		stmt->accept(*this);
	}
	return false;
}
#endif
