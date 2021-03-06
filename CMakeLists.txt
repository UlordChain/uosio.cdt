cmake_minimum_required(VERSION 3.5)
project(uosio_cdt)

find_program(SCCACHE_FOUND sccache)
if (SCCACHE_FOUND)
   message(STATUS "Using sccache")
   set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE sccache)
else()
   find_program(CCACHE_FOUND ccache)
   if (CCACHE_FOUND)
      message(STATUS "Using ccache")
      set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
   endif()
endif()


set(VERSION_MAJOR 1)
set(VERSION_MINOR 6)
set(VERSION_PATCH 2)
#set(VERSION_SUFFIX rc2)

if (VERSION_SUFFIX)
   set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_SUFFIX}")
else()
   set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

set(CDT_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/uosio.cdt)

include(GNUInstallDirs)

include(modules/ClangExternalProject.txt)
include(modules/ToolsExternalProject.txt)

set(WASM_SDK_BUILD true)

### Configure the UosioWasmToolchain.cmakes
set(CDT_ROOT_DIR ${CMAKE_BINARY_DIR})
configure_file(${CMAKE_SOURCE_DIR}/modules/uosio.cdt-config.cmake ${CMAKE_BINARY_DIR}/lib/cmake/uosio.cdt/uosio.cdt-config.cmake @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/modules/UosioCDTMacros.cmake.in ${CMAKE_BINARY_DIR}/lib/cmake/uosio.cdt/UosioCDTMacros.cmake @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/modules/UosioWasmToolchain.cmake.in ${CMAKE_BINARY_DIR}/lib/cmake/uosio.cdt/UosioWasmToolchain.cmake @ONLY)

set(CDT_ROOT_DIR ${CDT_INSTALL_PREFIX})
configure_file(${CMAKE_SOURCE_DIR}/modules/uosio.cdt-config.cmake ${CMAKE_BINARY_DIR}/modules/uosio.cdt-config.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/modules/uosio.cdt-config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/uosio.cdt)
configure_file(${CMAKE_SOURCE_DIR}/modules/UosioCDTMacros.cmake.in ${CMAKE_BINARY_DIR}/modules/UosioCDTMacros.cmake @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/modules/UosioWasmToolchain.cmake.in ${CMAKE_BINARY_DIR}/modules/UosioWasmToolchain.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/modules/UosioWasmToolchain.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/uosio.cdt)
install(FILES ${CMAKE_BINARY_DIR}/modules/UosioCDTMacros.cmake DESTINATION ${CDT_INSTALL_PREFIX}/lib/cmake/uosio.cdt)

set(CDT_ROOT_DIR "_PREFIX_")
configure_file(${CMAKE_SOURCE_DIR}/modules/UosioCDTMacros.cmake.in ${CMAKE_BINARY_DIR}/modules/UosioCDTMacrosPackage.cmake @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/modules/UosioWasmToolchain.cmake.in ${CMAKE_BINARY_DIR}/modules/UosioWasmToolchainPackage.cmake @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/modules/uosio.cdt-config.cmake ${CMAKE_BINARY_DIR}/modules/uosio.cdt-config.cmake.package @ONLY)

include(modules/LibrariesExternalProject.txt)

include(modules/InstallCDT.cmake)

configure_file(${CMAKE_SOURCE_DIR}/imports/uosio.imports.in ${CMAKE_BINARY_DIR}/uosio.imports COPYONLY)
install(FILES ${CMAKE_BINARY_DIR}/uosio.imports DESTINATION ${CDT_INSTALL_PREFIX})

configure_file(${CMAKE_SOURCE_DIR}/scripts/ricarduos/ricarduos.py ${CMAKE_BINARY_DIR}/scripts/ricarduos.py COPYONLY)
install(FILES ${CMAKE_BINARY_DIR}/scripts/ricarduos.py DESTINATION ${CDT_INSTALL_PREFIX}/scripts)

# section for package construction
set(VENDOR "block.one")
set(PROJECT_NAME "uosio.cdt")
set(DESC "Toolchain and supporting tools for the UOS.IO platform")
set(URL "https://github.com/uosio/uosio.cdt")
set(EMAIL "support@block.one")
configure_file(${CMAKE_SOURCE_DIR}/scripts/generate_package.sh.in ${CMAKE_BINARY_DIR}/packages/generate_package.sh @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/scripts/generate_deb.sh ${CMAKE_BINARY_DIR}/packages/generate_deb.sh COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/scripts/generate_bottle.sh ${CMAKE_BINARY_DIR}/packages/generate_bottle.sh COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/scripts/generate_rpm.sh ${CMAKE_BINARY_DIR}/packages/generate_rpm.sh COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/scripts/generate_tarball.sh ${CMAKE_BINARY_DIR}/packages/generate_tarball.sh COPYONLY)

# add licenses
configure_file(${CMAKE_SOURCE_DIR}/uosio_llvm/LICENSE.TXT ${CMAKE_BINARY_DIR}/licenses/llvm.license COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/libraries/boost/boost.license ${CMAKE_BINARY_DIR}/licenses/boost.license COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tools/external/wabt/LICENSE ${CMAKE_BINARY_DIR}/licenses/wabt.license COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tools/jsoncons/LICENSE ${CMAKE_BINARY_DIR}/licenses/jsoncons.license COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_BINARY_DIR}/licenses/uosio.cdt.license COPYONLY)

include(modules/TestsExternalProject.txt)

include(CTest)
enable_testing()
add_subdirectory(tests)
