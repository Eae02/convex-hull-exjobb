#!/bin/bash

set -e 0

if [[ $1 == "debug" ]]; then
	BUILD_TYPE="Debug"
else
	BUILD_TYPE="Release"
fi

mkdir -p .build/$BUILD_TYPE
cmake -B .build/$BUILD_TYPE -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build .build/$BUILD_TYPE --parallel
