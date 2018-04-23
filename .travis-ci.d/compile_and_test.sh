#!/bin/bash

ls -la

source dependencies/root/bin/thisroot.sh

mkdir -p build
cd build

cmake -DDQM4HEP_DOXYGEN_DOC=OFF -DCMAKE_MODULE_PATH=$PWD/../dependencies/dqm4hep/cmake -DDQMCore_DIR=$PWD/../dependencies/dqm4hep-core -DDQMNet_DIR=$PWD/../dependencies/dqm4hep-net -DDQM4HEP_TESTING=ON -DDQM4HEP_WARNING_AS_ERROR=ON -DDQM4HEP_DEV_WARNINGS=ON ..

if [ $? -ne 0 ]; then
    echo "Failed to run cmake"
    exit 1
fi

make install VERBOSE=1

if [ $? -ne 0 ]; then
    echo "Failed to run make"
    exit 1
fi

ldd ./lib/*.so
ldd ./bin/*
#
# ctest -V
#
# if [ $? -ne 0 ]; then
#     echo "Failed to run cmake tests"
#     exit 1
# fi
