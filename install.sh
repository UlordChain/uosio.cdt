#!/bin/bash
##########################################################################
# This is the UOSIO automated install script for Linux and Mac OS.
# This file was downloaded from https://github.com/UOSIO/uos
#
# Copyright (c) 2017, Respective Authors all rights reserved.
#
# After June 1, 2018 this software is available under the following terms:
# 
# The MIT License
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# https://github.com/UOSIO/uos/blob/master/LICENSE.txt
##########################################################################
   

	CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
	if [ "${CWD}" != "${PWD}" ]; then
		printf "\\n\\tPlease cd into directory %s to run this script.\\n \\tExiting now.\\n\\n" "${CWD}"
		exit 1
	fi

	BUILD_DIR="${PWD}/build"
	CMAKE_BUILD_TYPE=Release
	TIME_BEGIN=$( date -u +%s )
   INSTALL_PREFIX="/usr/local/uosio.cdt"
	VERSION=1.2

	txtbld=$(tput bold)
	bldred=${txtbld}$(tput setaf 1)
	txtrst=$(tput sgr0)

   create_symlink() {
      pushd /usr/local/bin &> /dev/null
      ln -sf ../uosio.cdt/bin/$1 $2
      popd &> /dev/null
   }

   install_symlinks() {
		printf "\\n\\tInstalling UOSIO.CDT Binary Symlinks\\n\\n"
      create_symlink "llvm-ranlib uosio-ranlib"
      create_symlink "llvm-ar uosio-ar"
      create_symlink "llvm-objdump uosio-objdump"
      create_symlink "llvm-readelf uosio-readelf"
      create_symlink "uosio-cc uosio-cc"
      create_symlink "uosio-cpp uosio-cpp"
      create_symlink "uosio-ld uosio-ld"
      create_symlink "uosio-pp uosio-pp"
      create_symlink "uosio-init uosio-init"
      create_symlink "uosio-abigen uosio-abigen"
      create_symlink "uosio-abidiff uosio-abidiff"
      create_symlink "uosio-wasm2wast uosio-wasm2wast"
      create_symlink "uosio-wast2wasm uosio-wast2wasm"
   }
   
   create_cmake_symlink() {
      mkdir -p /usr/local/lib/cmake/uosio.cdt
      pushd /usr/local/lib/cmake/uosio.cdt &> /dev/null
      ln -sf ../../../uosio.cdt/lib/cmake/uosio.cdt/$1 $1
      popd &> /dev/null
   }
	if [ ! -d "${BUILD_DIR}" ]; then
      printf "\\n\\tError, build.sh has not ran.  Please run ./build.sh first!\\n\\n"
      exit -1
	fi

	if ! pushd "${BUILD_DIR}"
	then
		printf "Unable to enter build directory %s.\\n Exiting now.\\n" "${BUILD_DIR}"
		exit 1;
	fi
   
	if ! make install
	then
		printf "\\n\\t>>>>>>>>>>>>>>>>>>>> MAKE installing UOSIO has exited with the above error.\\n\\n"
		exit -1
	fi
   popd &> /dev/null 

   install_symlinks   
   create_cmake_symlink "uosio.cdt-config.cmake"

   printf "\n\n${bldred}\n"
   printf "\t _     _   _______   _______  _________  _______\n"
   printf "\t( )   ( ) (  ___  ) (  ____ \ \___ ___/ (  ___  )\n"
   printf "\t| |   | | | (   ) | | (    \/    ) (    | (   ) |\n"
   printf "\t| |   | | | |   | | | (_____     | |    | |   | |\n"
   printf "\t| |   | | | |   | | (_____  )    | |    | |   | |\n"
   printf "\t| |   | | | |   | |       ) |    | |    | |   | |\n"
   printf "\t| (___) | | (___) | /\____) | ___) (___ | (___) |\n"
   printf "\t(_______) (_______) \_______) \_______/ (_______)\n${txtrst}"


	printf "\\tFor more information:\\n"
	printf "\\tUOSIO website: https://uos.io\\n"
