cmake_minimum_required(VERSION 3.5)
project(dice_example VERSION 1.0.0)

find_package(uosio.cdt)

### Generate the wasm and abi
add_contract( dice dice dice.cpp )

link_directories(/usr/local/uosio.cdt/lib)

#set(IMPORTLIB  libc++ libc uosiolib
#)

### add the path to where the ricardian contracts/clauses are found
#target_compile_options( dice.wasm PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR} ${IMPORTLIB}  )
target_compile_options( dice.wasm PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR} )
