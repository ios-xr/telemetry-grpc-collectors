#!/bin/bash
#
# Copyright (c) 2016 by cisco Systems, Inc. 
# All rights reserved.
#
set -x

if [ -d src/gen-obj ]; then
  rm -r src/gen-obj
fi

if [ -d src/gen-cpp ]; then
  rm -r src/gen-cpp
fi

mkdir -p src/gen-cpp
mkdir -p src/gen-obj
printf "Generating cplusplus bindings..."

cd ./proto/

for dir in `find . -type d -links 2`; do
  protoc -I ./  --grpc_out=../src/gen-cpp --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $dir/*.proto
  protoc -I ./ --cpp_out=../src/gen-cpp $dir/*.proto
  mkdir -p ../src/gen-obj/$dir
done

for file in .; do
  protoc -I ./  --grpc_out=../src/gen-cpp --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $file/*.proto
  protoc -I ./ --cpp_out=../src/gen-cpp $file/*.proto
done

echo "Done"
