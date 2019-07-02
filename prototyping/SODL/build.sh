#!/bin/bash

# optional parameters:
#
# --release		perform a RELEASE build instead of DEBUG

BUILD_TYPE="Debug"
if [[ $1 = "--release" ]]; then
	BUILD_TYPE="Release"
fi

if [ ! -d build ]; then
    mkdir build
fi
cd build
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -G "Unix Makefiles" ..
RESULT=$?
if [[ $RESULT != 0 ]]; then
	printf "\nCMake error. exiting.\n"
	exit 1
fi
make -j4
RESULT=$?
cd ..

if [[ $RESULT = 0 ]]; then
	cp build/SODL_test* ./
	printf "\n Success.\n\n"
	exit 0
else
	printf "\n Errors encountered. \n\n"
	exit 1
fi
