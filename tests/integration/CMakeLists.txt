cmake_minimum_required( VERSION 3.5 )

set(UOSIO_VERSION_MIN "1.4")
set(UOSIO_VERSION_SOFT_MAX "1.4")
#set(UOSIO_VERSION_HARD_MAX "")

find_package(uosio)

### Check the version of uosio
set(VERSION_MATCH_ERROR_MSG "")
UOSIO_CHECK_VERSION(VERSION_OUTPUT "${UOSIO_VERSION}"
                                   "${UOSIO_VERSION_MIN}"
                                   "${UOSIO_VERSION_SOFT_MAX}"
                                   "${UOSIO_VERSION_HARD_MAX}"
                                   VERSION_MATCH_ERROR_MSG)
if(VERSION_OUTPUT STREQUAL "MATCH")
   message(STATUS "Using uosio version ${UOSIO_VERSION}")
elseif(VERSION_OUTPUT STREQUAL "WARN")
   message(WARNING "Using uosio version ${UOSIO_VERSION} even though it exceeds the maximum supported version of ${UOSIO_VERSION_SOFT_MAX}; continuing with configuration, however build may fail.\nIt is recommended to use uosio version ${UOSIO_VERSION_SOFT_MAX}.x")
else() # INVALID OR MISMATCH
   message(FATAL_ERROR "Found uosio version ${UOSIO_VERSION} but it does not satisfy version requirements: ${VERSION_MATCH_ERROR_MSG}\nPlease use uosio version ${UOSIO_VERSION_SOFT_MAX}.x")
endif(VERSION_OUTPUT STREQUAL "MATCH")


enable_testing()

configure_file(${CMAKE_SOURCE_DIR}/contracts.hpp.in ${CMAKE_BINARY_DIR}/contracts.hpp)

include_directories(${CMAKE_BINARY_DIR})

file(GLOB INT_TESTS "*.cpp" "*.hpp")

add_uosio_test( integration_tests ${INT_TESTS} )
