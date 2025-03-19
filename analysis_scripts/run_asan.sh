#!/bin/bash

if [[ "$(pwd)" == */analysis_scripts ]]; then
  cd ..
fi

mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON ..
cmake --build .

./tests/unit_tests/CppGit_unit_tests
./tests/integration_tests/CppGit_integration_tests