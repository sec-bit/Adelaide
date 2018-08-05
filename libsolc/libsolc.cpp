/*
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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * JSON interface for the solidity compiler to be used from Javascript.
 */

#include <libsolc/libsolc.h>
#include <libdevcore/Common.h>
#include <libdevcore/JSON.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/Version.h>

#ifdef SECBIT
#include <boost/algorithm/string.hpp>
#endif
#include <string>

#include "license.h"

using namespace std;
using namespace dev;
using namespace solidity;

namespace
{

ReadCallback::Callback wrapReadCallback(CStyleReadFileCallback _readCallback = nullptr)
{
	ReadCallback::Callback readCallback;
	if (_readCallback)
	{
		readCallback = [=](string const& _path)
		{
			char* contents_c = nullptr;
			char* error_c = nullptr;
			_readCallback(_path.c_str(), &contents_c, &error_c);
			ReadCallback::Result result;
			result.success = true;
			if (!contents_c && !error_c)
			{
				result.success = false;
				result.responseOrErrorMessage = "File not found.";
			}
			if (contents_c)
			{
				result.success = true;
				result.responseOrErrorMessage = string(contents_c);
				free(contents_c);
			}
			if (error_c)
			{
				result.success = false;
				result.responseOrErrorMessage = string(error_c);
				free(error_c);
			}
			return result;
		};
	}
	return readCallback;
}

#ifdef SECBIT
string compile(StringMap const& _sources, bool _optimize, CStyleReadFileCallback _readCallback,
	       bool _isSECBIT = false, bool _noSMT = false, bool _asERC20 = false, char const* _tags = nullptr)
#else
/// Translates a gas value as a string to a JSON number or null
Json::Value gasToJson(Json::Value const& _value)
{
	if (_value.isObject())
	{
		Json::Value ret = Json::objectValue;
		for (auto const& sig: _value.getMemberNames())
			ret[sig] = gasToJson(_value[sig]);
		return ret;
	}

	if (_value == "infinite")
		return Json::Value(Json::nullValue);

	u256 value(_value.asString());
	if (value > std::numeric_limits<Json::LargestUInt>::max())
		return Json::Value(Json::nullValue);
	else
		return Json::Value(Json::LargestUInt(value));
}

Json::Value translateGasEstimates(Json::Value const& estimates)
{
	Json::Value output(Json::objectValue);

	if (estimates["creation"].isObject())
	{
		Json::Value creation(Json::arrayValue);
		creation[0] = gasToJson(estimates["creation"]["executionCost"]);
		creation[1] = gasToJson(estimates["creation"]["codeDepositCost"]);
		output["creation"] = creation;
	}
	else
		output["creation"] = Json::objectValue;
	output["external"] = gasToJson(estimates.get("external", Json::objectValue));
	output["internal"] = gasToJson(estimates.get("internal", Json::objectValue));

	return output;
}

string compile(StringMap const& _sources, bool _optimize, CStyleReadFileCallback _readCallback)
#endif
{
	/// create new JSON input format
	Json::Value input = Json::objectValue;
	input["language"] = "Solidity";
	input["sources"] = Json::objectValue;
	for (auto const& source: _sources)
	{
		input["sources"][source.first] = Json::objectValue;
		input["sources"][source.first]["content"] = source.second;
	}
	input["settings"] = Json::objectValue;
	input["settings"]["optimizer"] = Json::objectValue;
	input["settings"]["optimizer"]["enabled"] = _optimize;
	input["settings"]["optimizer"]["runs"] = 200;

#ifdef SECBIT
	input["settings"]["secbit"] = Json::objectValue;
	input["settings"]["secbit"]["enabled"] = _isSECBIT;
	input["settings"]["secbit"]["noSMT"] = _noSMT;
	input["settings"]["secbit"]["asERC20"] = _asERC20;
	Json::Value secbitTags = Json::arrayValue;
	if(_tags) {
		vector<string> tags;
		boost::split(tags, _tags, boost::is_any_of(","));
		for (string const& tag: tags) {
			secbitTags.append(tag);
		}
	}
	input["settings"]["secbit"]["tag"] = secbitTags;
#endif
	// Enable all SourceUnit-level outputs.
	input["settings"]["outputSelection"]["*"][""][0] = "*";
	// Enable all Contract-level outputs.
	input["settings"]["outputSelection"]["*"]["*"][0] = "*";

	StandardCompiler compiler(wrapReadCallback(_readCallback));
	Json::Value ret = compiler.compile(input);

	/// transform JSON to match the old format
	// {
	//   "errors": [ "Error 1", "Error 2" ],
	//   "sourceList": [ "sourcename1", "sourcename2" ],
	//   "sources": {
	//     "sourcename1": {
	//       "AST": {}
	//     }
	//   },
	//   "contracts": {
	//     "Contract1": {
	//       "interface": "[...abi...]",
	//       "bytecode": "ff0011...",
	//       "runtimeBytecode": "ff0011",
	//       "opcodes": "PUSH 1 POP STOP",
	//       "metadata": "{...metadata...}",
	//       "functionHashes": {
	//         "test(uint256)": "11ff2233"
	//       },
	//       "gasEstimates": {
	//         "creation": [ 224, 42000 ],
	//         "external": {
	//           "11ff2233": null,
	//           "3322ff11": 1234
	//         },
	//         "internal": {
	//         }
	//       },
	//       "srcmap" = "0:1:2",
	//       "srcmapRuntime" = "0:1:2",
	//       "assembly" = {}
	//     }
	//   }
	// }
	Json::Value output = Json::objectValue;

#ifdef SECBIT
	output["errors"] = ret["errors"];
#else
	if (ret.isMember("errors"))
	{
		output["errors"] = Json::arrayValue;
		for (auto const& error: ret["errors"])
			output["errors"].append(
				!error["formattedMessage"].empty() ? error["formattedMessage"] : error["message"]
			);
	}

	output["sourceList"] = Json::arrayValue;
	for (auto const& source: _sources)
		output["sourceList"].append(source.first);

	if (ret.isMember("sources"))
	{
		output["sources"] = Json::objectValue;
		for (auto const& sourceName: ret["sources"].getMemberNames())
		{
			output["sources"][sourceName] = Json::objectValue;
			output["sources"][sourceName]["AST"] = ret["sources"][sourceName]["legacyAST"];
		}
	}

	if (ret.isMember("contracts"))
	{
		output["contracts"] = Json::objectValue;
		for (auto const& sourceName: ret["contracts"].getMemberNames())
			for (auto const& contractName: ret["contracts"][sourceName].getMemberNames())
			{
				Json::Value contractInput = ret["contracts"][sourceName][contractName];
				Json::Value contractOutput = Json::objectValue;
				contractOutput["interface"] = jsonCompactPrint(contractInput["abi"]);
				contractOutput["metadata"] = contractInput["metadata"];
				contractOutput["functionHashes"] = contractInput["evm"]["methodIdentifiers"];
				contractOutput["gasEstimates"] = translateGasEstimates(contractInput["evm"]["gasEstimates"]);
				contractOutput["assembly"] = contractInput["evm"]["legacyAssembly"];
				contractOutput["bytecode"] = contractInput["evm"]["bytecode"]["object"];
				contractOutput["opcodes"] = contractInput["evm"]["bytecode"]["opcodes"];
				contractOutput["srcmap"] = contractInput["evm"]["bytecode"]["sourceMap"];
				contractOutput["runtimeBytecode"] = contractInput["evm"]["deployedBytecode"]["object"];
				contractOutput["srcmapRuntime"] = contractInput["evm"]["deployedBytecode"]["sourceMap"];
				output["contracts"][sourceName + ":" + contractName] = contractOutput;
			}
	}
#endif

	try
	{
		return jsonCompactPrint(output);
	}
	catch (...)
	{
		return "{\"errors\":[\"Unknown error while generating JSON.\"]}";
	}
}

string compileMulti(string const& _input, bool _optimize, CStyleReadFileCallback _readCallback = nullptr)
{
	string errors;
	Json::Value input;
	if (!jsonParseStrict(_input, input, &errors))
	{
		Json::Value jsonErrors(Json::arrayValue);
		jsonErrors.append("Error parsing input JSON: " + errors);
		Json::Value output(Json::objectValue);
		output["errors"] = jsonErrors;
		return jsonCompactPrint(output);
	}
	else
	{
		StringMap sources;
		Json::Value jsonSources = input["sources"];
		if (jsonSources.isObject())
			for (auto const& sourceName: jsonSources.getMemberNames())
				sources[sourceName] = jsonSources[sourceName].asString();
		return compile(sources, _optimize, _readCallback);
	}
}

#ifdef SECBIT
string compileSingle(string const& _input, bool _optimize, bool _isSECBIT, bool _noSMT, bool _asERC20, char const* _tags)
#else
string compileSingle(string const& _input, bool _optimize)
#endif
{
	StringMap sources;
	sources[""] = _input;
#ifdef SECBIT
	return compile(sources, _optimize, nullptr, _isSECBIT, _noSMT, _asERC20, _tags);
#else
	return compile(sources, _optimize, nullptr);
#endif
}


string compileStandardInternal(string const& _input, CStyleReadFileCallback _readCallback = nullptr)
{
	StandardCompiler compiler(wrapReadCallback(_readCallback));
	return compiler.compile(_input);
}

}

static string s_outputBuffer;

extern "C"
{
extern char const* license()
{
	static string fullLicenseText = otherLicenses + licenseText;
	return fullLicenseText.c_str();
}
extern char const* version()
{
	return VersionString.c_str();
}
#ifdef SECBIT
extern char const* compileJSON(char const* _input, bool _optimize, bool _isSECBIT, bool _noSMT, bool _asERC20, char const* _tags)
{
	s_outputBuffer = compileSingle(_input, _optimize, _isSECBIT, _noSMT, _asERC20, _tags);
	return s_outputBuffer.c_str();
}
#else
extern char const* compileJSON(char const* _input, bool _optimize)
{
	s_outputBuffer = compileSingle(_input, _optimize);
	return s_outputBuffer.c_str();
}
#endif
extern char const* compileJSONMulti(char const* _input, bool _optimize)
{
	s_outputBuffer = compileMulti(_input, _optimize);
	return s_outputBuffer.c_str();
}
extern char const* compileJSONCallback(char const* _input, bool _optimize, CStyleReadFileCallback _readCallback)
{
	s_outputBuffer = compileMulti(_input, _optimize, _readCallback);
	return s_outputBuffer.c_str();
}
extern char const* compileStandard(char const* _input, CStyleReadFileCallback _readCallback)
{
	s_outputBuffer = compileStandardInternal(_input, _readCallback);
	return s_outputBuffer.c_str();
}
}
