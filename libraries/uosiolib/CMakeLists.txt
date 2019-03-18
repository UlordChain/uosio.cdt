file(GLOB HEADERS "*.hpp"
                  "*.h")

add_library(uosio
            uosiolib.cpp
            crypto.cpp
            ${HEADERS})

add_native_library(native_uosio
                   uosiolib.cpp
                   crypto.cpp
                   ${HEADERS})

target_include_directories(uosio PUBLIC
                                 ${CMAKE_SOURCE_DIR}/libc/musl/include
                                 ${CMAKE_SOURCE_DIR}/libc/musl/src/internal
                                 ${CMAKE_SOURCE_DIR}/libc/musl/src/crypt
                                 ${CMAKE_SOURCE_DIR}/libc/musl/arch/uos
                                 {CMAKE_SOURCE_DIR}/libcxx/include
                                 ${CMAKE_SOURCE_DIR}
                                 ${CMAKE_SOURCE_DIR}/boost/include)

target_include_directories(native_uosio PUBLIC
                                 ${CMAKE_SOURCE_DIR}/libc/musl/include
                                 ${CMAKE_SOURCE_DIR}/libc/musl/src/internal
                                 ${CMAKE_SOURCE_DIR}/libc/musl/src/crypt
                                 ${CMAKE_SOURCE_DIR}/libc/musl/arch/uos
                                 ${CMAKE_SOURCE_DIR}/libcxx/include
                                 ${CMAKE_SOURCE_DIR}
                                 ${CMAKE_SOURCE_DIR}/boost/include)

target_link_libraries( uosio c c++ )
target_link_libraries( native_uosio native_c native_c++ )
add_dependencies( native_uosio uosio )


add_custom_command( TARGET uosio POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:uosio> ${BASE_BINARY_DIR}/lib )
add_custom_command( TARGET native_uosio POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:native_uosio> ${BASE_BINARY_DIR}/lib )

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../uosiolib DESTINATION ${BASE_BINARY_DIR}/include FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
