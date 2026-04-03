#!/bin/sh
make clean
clang-format -i $(find . -name "*.cpp" -o -name "*.hpp" -o -name "*.h")
make
make test
