#!/bin/bash

cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_UBSAN=ON ..
cmake --build .

./tests/unit_tests/CppGit_unit_tests
./tests/integration_tests/CppGit_integration_tests