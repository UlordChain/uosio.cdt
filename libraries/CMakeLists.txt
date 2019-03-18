project(uosio_libraries)

list(APPEND CMAKE_MODULE_PATH ${UOSIO_CDT_BIN})
include(UosioCDTMacros)

set(CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} -O3 -Wall ")
set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -O3 -Wall ")

add_subdirectory(libc)
add_subdirectory(libc++)
add_subdirectory(uosiolib)
add_subdirectory(boost)
add_subdirectory(native)
