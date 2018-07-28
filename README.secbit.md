# The SECBIT Static Analysis Extension to Solidity Compiler

This repository is forked from the [Solidity Compiler](https://github.com/ethereum/solidity).

The purpose is to extend the Solidity Compiler `solc` with additional static analysis capabilities.

## Build

Please follow the same instruction for building Solidity compiler to build the SECBIT-extended `solc`.

## Usage

### Command Line

The basic command line to invoke all checks is:
```
$ solc --secbit-warnings output-file -o output-dir --overwrite input-file.sol
```
...where `input-file.sol` is the Solidity source file, and `output-dir/output-file` is the the JSON output. For example,
```JavaScript
{
  "secbit-warnings" : 
  [
    {
      "desc" : "This ERC20 function returns false, which may not be correctly handled by the caller.",
      "endcolumn" : 3,
      "endline" : 59,
      "file" : "test.sol",
      "startcolumn" : 2,
      "startline" : 57,
      "tag" : "erc20-return-false"
    },
    {
      "desc" : "Missing check on 'msg.data.length' could lead to short-address attack in this ERC20 transfer function.",
      "endcolumn" : 3,
      "endline" : 59,
      "file" : "test.sol",
      "startcolumn" : 2,
      "startline" : 57,
      "tag" : "short-addr"
    }
  ]
}
```

To turn off the experimental SMT-based checks, `--no-smt` should be used.
```
$ solc --secbit-warnings output-file -o output-dir --overwrite --no-smt input-file
```

To only run specific checks, one or more `--secbit-tag` could be used.
```
$ solc --secbit-warnings output-file -o output-dir --overwrite --secbit-tag bad-name input-file
```
```
$ solc --secbit-warnings output-file -o output-dir --overwrite --secbit-tag bad-name --secbit-tag erc20-no-return input-file
```

To only run ERC20-specific checks, `--erc20` should be used.
```
$ solc --secbit-warnings output-file -o output-dir --overwrite --erc20 input-file
```
...this effectively turns on all ERC20-specific checks.

If the active checks do not contain SMT-related checks (`reentrance` and `unchecked-math`), SMT solver will be turned off.

### Visual Studio Code Extension

SECBIT Labs also provides an [Visual Studio Code](https://code.visualstudio.com/) Extension for running SECBIT
Solidity Static Analysis over Solidity source code in the IDE.

The extension could be find at:
https://github.com/ethereum/vscode-secbit-sa


## Modifications

Source code modifications from SECBIT are enclosed in `#ifdef SECBIT` directives.

The two new files added are:
`libsolidity/analysis/SECBITChecker.h`
`libsolidity/analysis/SECBITChecker.cpp`

## License

SECBIT Labs retains copyright to all above mentioned source code modifications. And the modifications
are licensed under the same license as Solidity.
