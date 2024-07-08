#! /bin/bash

# export LD_LIBRARY_PATH=/FastDDS/build/src/cpp:$LD_LIBRARY_PATH
# export FASTRTPS_DEFAULT_PROFILES_FILE=/FastDDS/examples/cpp/dds/HelloFastDDS/HelloFastDDSqos.xml
cmake -H. -Bbuild -DCOMPILE_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build ./build --parallel 8
