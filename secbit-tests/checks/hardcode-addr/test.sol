pragma solidity 0.4.16;

contract C {
    function addr() public pure returns (address) {
        address multisig = 0xf64B584972FE6055a770477670208d737Fff282f;
        return multisig;
    }
    function addr_bad_checksum() public pure returns (address) {
        address multisig = 0x11aa000000000000000d00cc00000000000000bb;
        return multisig;
    }
}
