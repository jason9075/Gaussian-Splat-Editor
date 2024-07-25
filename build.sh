#!/user/bin/env bash

if [ "$1" == "-r" ]; then
    rm -rf build
fi

mkdir -p build
cd build || exit 1
cmake ..
make || exit 1
cd .. || exit 1

