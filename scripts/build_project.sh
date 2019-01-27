#!/usr/bin/env bash
# Copyright (c) 2019 Denis Trofimov (den.a.trofimov@yandex.ru)
# Distributed under the MIT License.
# See accompanying file LICENSE.md or copy at http://opensource.org/licenses/MIT

set -ev

# Possible config values are empty, Debug, Release, RelWithDebInfo and MinSizeRel.
cmake -E chdir build cmake -E time cmake .. -DBOOST_ROOT:STRING="/usr" -DENABLE_IWYU=OFF -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DENABLE_CODE_COVERAGE=OFF

cmake -E chdir build cmake -E time cmake --build . --config Debug --clean-first -- -j8

cmake -E chdir build cmake -E time cmake --build . --config Debug --target run_all_tests
