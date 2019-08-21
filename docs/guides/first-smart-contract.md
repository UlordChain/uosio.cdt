### Building your first smart contract
```c++
#include <uosio/uosio.hpp>
#include <uosio/name.hpp>

class [[uosio::contract]] hello : public uosio::contract {
   public:
      using uosio::contract::contract;

      [[uosio::action]]
      void hi(uosio::name nm) {
         uosio::print_f("Hello, %\n", nm);
      }
};
```

- Navigate to the hello folder in examples (./examples/hello).
- You should then see the hello.cpp file
- Now run the compiler
```sh
$ uosio-cpp -abigen hello.cpp -o hello.wasm
```
- Or with CMake
```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
```
This will generate two files:
* The compiled binary wasm (hello.wasm)
* The generated ABI file (hello.abi)
