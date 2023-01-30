#!/bin/bash

if [[ $1 == "debug" ]]; then
	BUILD_TYPE="Debug"
else
	BUILD_TYPE="Release"
fi

set -e

mkdir -p .build/$BUILD_TYPE
cmake -B .build/$BUILD_TYPE -DCMAKE_BUILD_TYPE=$BUILD_TYPE # try adding: -D CMAKE_C_COMPILER=gcc-compiler -D CMAKE_CXX_COMPILER=g++-compiler
cmake --build .build/$BUILD_TYPE --parallel
