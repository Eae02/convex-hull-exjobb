#!/bin/bash

DIR=$(dirname -- "$0")
for f in $(find "$DIR/generators" -name "*.cpp"); do
	echo "building $f..."
	g++ --std=c++20 -O2 "$f" -o "$DIR/$(basename -- "$f" .cpp).bin"
done
