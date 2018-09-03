#!/bin/bash

SCRIPT_PATH=$(dirname `which $0`)

cd $SCRIPT_PATH
# Remove proto folder
rm -r ./proto

# Clean up the object files and library
make clean
