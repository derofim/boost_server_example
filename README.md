# About

WebSocket server- and client implementation built on top of Boost.Beast

# Supported platforms

Linux (tested on Ubuntu 18.04 LTS)

# Supported client platforms

TODO: godot https://github.com/godotengine/godot/pull/14888 & http://docs.godotengine.org/en/latest/classes/class_websocketclient.html#description

# Clone server

git clone https://gitlab.com/derofim/webrtc-test.git
git submodule update --init --recursive --depth 50

## DEPENDENCIES

### cmake

Install latest cmake (remove old before):
bash scripts/install_cmake.sh

### clang-format

sudo apt install clang-format

Integrate with your IDE ( QT instructions http://doc.qt.io/qtcreator/creator-beautifier.html )
Import .clang-format rules to IDE settings.

### boost

Remove old boost:
sudo apt-get remove libboost-system-dev libboost-program-options-dev libboost-all-dev -y
sudo apt-get autoremove
sudo rm -rf /usr/local/lib/libboost*
sudo rm -rf /usr/local/include/boost

Install new boost:
cd ~
wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.bz2 \
    && tar -xjf boost_1_69_0.tar.bz2 \
    && rm -rf boost_1_69_0.tar.bz2 \
    && cd boost_1_69_0 \
    && sudo ./bootstrap.sh --prefix=/usr/ \
    && sudo ./b2 link=shared install

cat /usr/include/boost/version.hpp

### include-what-you-use

see https://github.com/include-what-you-use/include-what-you-use

sudo apt-get install llvm-6.0-dev libclang-6.0-dev clang-6.0 -y

bash scripts/build_iwyu_submodule.sh

NOTE: change -DIWYU_LLVM_ROOT_PATH=/usr/lib/llvm-6.0 in build_iwyu_submodule.sh

NOTE: For Clang on Windows read https://metricpanda.com/rival-fortress-update-27-compiling-with-clang-on-windows

NOTE: don`t use "bits/*" or "*/details/*" includes, add them to mappings file (.imp)

read https://llvm.org/devmtg/2010-11/Silverstein-IncludeWhatYouUse.pdf
read https://github.com/include-what-you-use/include-what-you-use/tree/master/docs
read https://github.com/hdclark/Ygor/blob/master/artifacts/20180225_include-what-you-use/iwyu_how-to.txt

## BUILD main project (from root project dir)

bash scripts/rebuild_clean.sh

OR

bash scripts/build_project.sh

OR

bash scripts/release_project.sh

## RUN main project (from root project dir)

./build/bin/boostander

# Code coverage

sudo apt-get install lcov gcovr

scripts/code_coverage.sh

open build/coverage/index.html

NOTE: coverage requires g++/gcc


