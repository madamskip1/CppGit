#!/bin/bash

cd build

cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/tests/unit_tests/CppGit_unit_tests > /dev/null
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/tests/integration_tests/CppGit_integration_tests > /dev/null