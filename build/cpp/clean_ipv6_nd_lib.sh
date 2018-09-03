#!/bin/bash

SCRIPT_PATH=$(dirname `which $0`)

cd $SCRIPT_PATH

if [[ -d ./proto ]]; then
  # Remove proto folder
  rm -r ./proto
fi

cd $SCRIPT_PATH/ipv6-nd-make/
# Clean up the object files and library
make clean
