#!/usr/bin/env bash
# Copyright (c) 2019 Denis Trofimov (den.a.trofimov@yandex.ru)
# Distributed under the MIT License.
# See accompanying file LICENSE.md or copy at http://opensource.org/licenses/MIT

set -ev

# NOTE: code coverage requires g++/gcc

cmake -E remove_directory build

cmake -E make_directory build

# NOTE: change PATHS, such as DWEBRTC_SRC_PATH
cmake -E chdir build cmake -E time cmake .. -DCMAKE_C_COMPILER="gcc" -DCMAKE_CXX_COMPILER="g++" -DBOOST_ROOT:STRING="/usr" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DENABLE_IWYU=OFF -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DAUTORUN_TESTS=OFF -DENABLE_CODE_COVERAGE=ON

cmake -E chdir build cmake -E time cmake --build . --config Debug -- -j8
cmake -E chdir build cmake -E time cmake --build . --config Debug --target unit_tests_coverage
