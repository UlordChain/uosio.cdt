# UOSIO.CDT (Contract Development Toolkit)
## Version : 1.6.2

UOSIO.CDT is a toolchain for WebAssembly (WASM) and set of tools to facilitate contract writing for the UOSIO platform.  In addition to being a general purpose WebAssembly toolchain, [UOSIO](https://github.com/uosio/uos) specific optimizations are available to support building UOSIO smart contracts.  This new toolchain is built around [Clang 7](https://github.com/uosio/llvm), which means that UOSIO.CDT has the most currently available optimizations and analyses from LLVM, but as the WASM target is still considered experimental, some optimizations are not available or incomplete.

## Important!
UOSIO.CDT Version 1.3.x introduced quite a few breaking changes.  To have binary releases we needed to remove the concept of a core symbol from UOSIO.CDT. This meant drastic changes to symbol, asset and other types/functions that were connected to them. Since these changes would be disruptive, we decided to add as many disruptive changes needed for future contract writing, so that disruption should only occur once. Please read the **_Differences between Version 1.2.x and Version 1.3.x_** section of this readme.

### Binary Releases
UOSIO.CDT currently supports Mac OS X brew, Linux x86_64 Debian packages, and Linux x86_64 RPM packages.

**If you have previously installed UOSIO.CDT, please run the `uninstall` script (it is in the directory where you cloned UOSIO.CDT) before downloading and using the binary releases.**

#### Mac OS X Brew Install
```sh
$ brew tap uosio/uosio.cdt
$ brew install uosio.cdt
```
#### Mac OS X Brew Uninstall
```sh
$ brew remove uosio.cdt
```
#### Debian Package Install
```sh
$ wget https://github.com/uosio/uosio.cdt/releases/download/v1.6.2/uosio.cdt_1.6.2-1_amd64.deb
$ sudo apt install ./uosio.cdt_1.6.2-1_amd64.deb
```
#### Debian Package Uninstall
```sh
$ sudo apt remove uosio.cdt
```

#### Fedora RPM Package Install
```sh
$ wget https://github.com/uosio/uosio.cdt/releases/download/v1.6.2/uosio.cdt-1.6.2-1.fedora-x86_64.rpm
$ sudo yum install ./uosio.cdt-1.6.2-1.fedora-x86_64.rpm
```

#### Fedora RPM Package Uninstall
```sh
$ sudo yum remove uosio.cdt
```

#### Centos RPM Package Install
```sh
$ wget https://github.com/uosio/uosio.cdt/releases/download/v1.6.2/uosio.cdt-1.6.2-1.centos-x86_64.rpm
$ sudo yum install ./uosio.cdt-1.6.2-1.centos-x86_64.rpm
```

#### Centos RPM Package Uninstall
```sh
$ sudo yum remove uosio.cdt
```

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
* uosio-abidiff
* uosio-wasm2wast
* uosio-wast2wasm
* uosio-ranlib
* uosio-ar
* uosio-objdump
* uosio-readelf
