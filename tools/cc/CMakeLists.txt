configure_file(${CMAKE_CURRENT_SOURCE_DIR}/uosio-cc.cpp.in ${CMAKE_BINARY_DIR}/uosio-cc.cpp)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/uosio-cpp.cpp.in ${CMAKE_BINARY_DIR}/uosio-cpp.cpp)

add_tool(uosio-cc)
add_tool(uosio-cpp)
