file(GLOB HEADERS "*.hpp"
                  "*.h")

add_library(uosio
            uosiolib.cpp
            crypto.cpp
            ${HEADERS})

add_library(uosio_malloc
            malloc.cpp
            ${HEADERS})

add_library(uosio_dsm
            simple_malloc.cpp
            ${HEADERS})

add_library(uosio_cmem
            memory.cpp
            ${HEADERS})

add_native_library(native_uosio
                   uosiolib.cpp
                   crypto.cpp
                   malloc.cpp
                   ${HEADERS})

set_target_properties(uosio_malloc PROPERTIES LINKER_LANGUAGE C)

target_include_directories(uosio PUBLIC
                                 ${CMAKE_SOURCE_DIR}/libc/musl/include
                                 ${CMAKE_SOURCE_DIR}/libc/musl/src/internal
                                 ${CMAKE_SOURCE_DIR}/libc/musl/src/crypt
                                 ${CMAKE_SOURCE_DIR}/libc/musl/arch/uos
                                 ${CMAKE_SOURCE_DIR}/libcxx/include
                                 ${CMAKE_SOURCE_DIR}
                                 ${CMAKE_SOURCE_DIR}/boost/include)

target_link_libraries( uosio c c++ )
add_dependencies( native_uosio uosio )

add_custom_command( TARGET uosio POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:uosio> ${BASE_BINARY_DIR}/lib )
add_custom_command( TARGET uosio_malloc POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:uosio_malloc> ${BASE_BINARY_DIR}/lib )
add_custom_command( TARGET uosio_dsm POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:uosio_dsm> ${BASE_BINARY_DIR}/lib )
add_custom_command( TARGET uosio_cmem POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:uosio_cmem> ${BASE_BINARY_DIR}/lib )
add_custom_command( TARGET native_uosio POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:native_uosio> ${BASE_BINARY_DIR}/lib )

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../uosiolib DESTINATION ${BASE_BINARY_DIR}/include FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
