#! /bin/bash

cmake -H. -Bbuild -DCOMPILE_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build ./build --parallel 4
