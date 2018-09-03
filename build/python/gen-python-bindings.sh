#!/bin/bash
#
# Copyright (c) 2016 by cisco Systems, Inc. 
# All rights reserved.
#
set -x


SCRIPT_DIR="$(cd "$(dirname "${0}")"; echo "$(pwd)")"

# Removing old bindings
if [ -d ${SCRIPT_DIR}/src/genpy ]; then
  rm -rf ${SCRIPT_DIR}/src/genpy
fi

printf "Generating Python bindings..."
mkdir -p ${SCRIPT_DIR}/src/genpy

cd ../../bigmuddy-network-telemetry-proto/proto_archive/

for dir in `find . -type d -links 2`; do
  python -m grpc_tools.protoc -I ./ --python_out=${SCRIPT_DIR}/src/genpy/ --grpc_python_out=${SCRIPT_DIR}/src/genpy/ $dir/*.proto
  mkdir -p ${SCRIPT_DIR}/src/genpy/$dir
  touch ${SCRIPT_DIR}/src/genpy/${dir}/__init__.py
  2to3 -w ${SCRIPT_DIR}/src/genpy/${dir}/*.py
done

for file in .; do
  python -m grpc_tools.protoc -I ./ --python_out=${SCRIPT_DIR}/src/genpy/ --grpc_python_out=${SCRIPT_DIR}/src/genpy/ *.proto
  2to3 -w ${SCRIPT_DIR}/src/genpy/*.py
done

echo "Done"
