#!/bin/zsh

set -e

cmake -S. -Bbuild
cmake --build build -j10 
# pushd build && ctest --output-on-failure && popd
