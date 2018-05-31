#!/bin/bash

ls -la
mkdir -p dependencies && cd dependencies

# install root
wget https://root.cern.ch/download/root_v6.10.08.Linux-ubuntu14-x86_64-gcc4.8.tar.gz
tar -xf root_v6.10.08.Linux-ubuntu14-x86_64-gcc4.8.tar.gz
source root/bin/thisroot.sh
root-config --version

# install dqm4hep cmake macros
git clone https://github.com/dqm4hep/dqm4hep.git
export DQM4HEP_DIR=$PWD/dqm4hep

# install xdrstream
git clone https://github.com/dqm4hep/xdrstream.git
cd xdrstream
export XDRSTREAM_DIR=$PWD
mkdir build && cd build
cmake -DINSTALL_DOC=OFF ..

if [ $? -ne 0 ]; then
    echo "Failed to run xdrstream cmake"
    exit 1
fi

make install VERBOSE=1

if [ $? -ne 0 ]; then
    echo "Failed to run xdrstream make"
    exit 1
fi
cd ../..

# install dqm4hep-core
git clone https://github.com/dqm4hep/dqm4hep-core.git
cd dqm4hep-core
mkdir -p build && cd build
cmake -DDQM4HEP_DOXYGEN_DOC=OFF -Dxdrstream_DIR=$PWD/../../xdrstream/lib/cmake -DCMAKE_MODULE_PATH=$PWD/../../dqm4hep/cmake -DDQM4HEP_TESTING=OFF -DDQM4HEP_WARNING_AS_ERROR=ON -DDQM4HEP_DEV_WARNINGS=ON ..

if [ $? -ne 0 ]; then
    echo "Failed to run dqm4hep-core cmake"
    exit 1
fi

make install VERBOSE=1

if [ $? -ne 0 ]; then
    echo "Failed to run dqm4hep-core make"
    exit 1
fi
cd ../..

# install dqm4hep-net
git clone https://github.com/dqm4hep/dqm4hep-net.git
cd dqm4hep-net
mkdir -p build && cd build
<<<<<<< HEAD
cmake -DDQM4HEP_DOXYGEN_DOC=OFF -DCMAKE_MODULE_PATH=$PWD/../../dqm4hep/cmake -DDQMCore_DIR=$PWD/../../dqm4hep-core/lib/cmake -DDQM4HEP_TESTING=OFF -DDQM4HEP_WARNING_AS_ERROR=OFF -DDQM4HEP_DEV_WARNINGS=OFF ..
=======
cmake -DDQM4HEP_DOXYGEN_DOC=OFF -DCMAKE_MODULE_PATH=$PWD/../../dqm4hep/cmake -DDQMCore_DIR=$PWD/../../dqm4hep-core/lib/cmake -DDQM4HEP_TESTING=OFF -DDQM4HEP_WARNING_AS_ERROR=ON -DDQM4HEP_DEV_WARNINGS=ON ..
>>>>>>> Update travis scripts

if [ $? -ne 0 ]; then
    echo "Failed to run dqm4hep-net cmake"
    exit 1
fi

make install VERBOSE=1

if [ $? -ne 0 ]; then
    echo "Failed to run dqm4hep-net make"
    exit 1
fi
cd ../..

ls -la
cd ..
