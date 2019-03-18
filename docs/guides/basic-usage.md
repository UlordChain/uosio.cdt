
### Usage
---
#### CMakeLists.txt
To compile an UOSIO smart contract with CMake you'll need a CMake file. The template `CMakeLists.txt` in the examples folder is a good boilerplate.

For example:

In `CMakeLists.txt`:
```
cmake_minimum_required(VERSION 3.5)
project(test_example VERSION 1.0.0)

find_package(uosio.cdt)

add_contract( test test test.cpp )
```


In `test.cpp`:

```
#include <uosiolib/uosio.hpp>
using namespace uosio;

CONTRACT test : public uosio::contract {
public:
   using contract::contract;

   ACTION testact( name test ) {
   }
};

UOSIO_DISPATCH( test, (testact) )
```

To manually compile the source code, use [`uosio-cpp/uosio-cc`](/tools/uosio-cpp.html) and [`uosio-ld`](/tools/uosio-ld.html) as if it were __clang__ and __lld__. All the includes and options specific to UOSIO and CDT are baked in.