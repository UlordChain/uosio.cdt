# UOSIO.CDT (Contract Development Toolkit)
## Version : 1.5.0

UOSIO.CDT is a toolchain for WebAssembly (WASM) and set of tools to facilitate contract writing for the UOSIO platform.  In addition to being a general purpose WebAssembly toolchain, [UOSIO](https://github.com/uosio/uos) specific optimizations are available to support building UOSIO smart contracts.  This new toolchain is built around [Clang 7](https://github.com/uosio/llvm), which means that UOSIO.CDT has the most currently available optimizations and analyses from LLVM, but as the WASM target is still considered experimental, some optimizations are not available or incomplete.

### Guided Installation (Building from Scratch)
```sh
$ git clone --recursive https://github.com/uosio/uosio.cdt
$ cd uosio.cdt
$ ./build.sh
$ sudo ./install.sh
```

### Installed Tools
---
* uosio-cpp
* uosio-cc
* uosio-ld
* uosio-init
* uosio-abigen
* uosio-abidiff
* uosio-pp (post processing pass for WASM, automatically runs with uosio-cpp and uosio-ld)
* uosio-wasm2wast
* uosio-wast2wasm
* uosio-ranlib
* uosio-ar
* uosio-objdump
* uosio-readelf
