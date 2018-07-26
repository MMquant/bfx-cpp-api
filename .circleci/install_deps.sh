#!/usr/bin/env bash

# This script establishes custom dependencies directory.
echo "Creating ~/custom_deps directory"
mkdir $HOME/custom_deps
cd $HOME/custom_deps
echo "~/Adding custom_deps in $PATH"
export PATH=$PATH:$HOME/custom_deps
if ! [[ $HOME/custom_deps == *"$PATH"* ]]; then
    echo "Couldn't add custom_deps dir to $PATH"
    exit 1
fi

## CMake
echo "Installing CMake..."
curl -sS -o cmake.tar.gz https://cmake.org/files/v3.12/cmake-3.12.0-Linux-x86_64.tar.gz
echo "Extracting CMake..."
# delete archive so that it's not being cache "checksummed" by circleci
tar -xf cmake.tar.gz && rm cmake.tar.gz
ln -s cmake-3.12.0-Linux-x86_64/bin/cmake cmake
echo "Done!"
