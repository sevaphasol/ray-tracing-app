#!/bin/bash

cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install -B build --verbose

pushd build
make -j$(nproc) install
popd

ln -sf build/compile_commands.json
