#!/usr/bin/env bash

# This script establishes custom dependencies directory.
mkdir $HOME/custom_deps
export PATH=$PATH:$HOME/custom_deps
cd $HOME/custom_deps

# CMake
curl -o cmake.tar.gz https://cmake.org/files/v3.12/cmake-3.12.0-Linux-x86_64.tar.gz
tar -xf cmake.tar.gz
ln -s cmake-3.12.0-Linux-x86_64/cmake cmake