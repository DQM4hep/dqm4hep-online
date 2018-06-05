#!/bin/bash

ls -la
mkdir -p dependencies && cd dependencies

# install root
if [[ "${TRAVIS_OS_NAME}" == "linux" ]]; then export ROOT_BUILD="Linux-ubuntu14-x86_64-gcc4.8";
elif [[ "${TRAVIS_OS_NAME}" == "osx" ]]; then export ROOT_BUILD="macosx64-";
    if [[ "${TRAVIS_OSX_IMAGE}" == "xcode8.3" ]]; then export ROOT_BUILD=$ROOT_BUILD"10.12-clang81";
    elif [[ "${TRAVIS_OSX_IMAGE}" == "xcode9.3" ]]; then export ROOT_BUILD=$ROOT_BUILD"10.13-clang90";
    fi;
fi
export ROOT_BIN="root_v${ROOT_VERSION}.${ROOT_BUILD}.tar.gz"
wget https://root.cern.ch/download/${ROOT_BIN}
tar -xf ${ROOT_BIN}
source root/bin/thisroot.sh
root-config --version
cd ..
