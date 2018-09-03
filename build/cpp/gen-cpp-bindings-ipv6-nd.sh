#!/bin/bash
#
# Copyright (c) 2016 by cisco Systems, Inc. 
# All rights reserved.
#
set -x

if [ -d src/gen-ipv6-nd-obj ]; then
  rm -r src/gen-ipv6-nd-obj
fi

if [ -d src/gen-ipv6-nd-cpp ]; then
  rm -r src/gen-ipv6-nd-cpp
fi

mkdir -p src/gen-ipv6-nd-cpp
mkdir -p src/gen-ipv6-nd-obj
printf "Generating cplusplus bindings..."

cd ./proto/

for dir in `find . -type d -links 2`; do
  protoc -I ./  --grpc_out=../src/gen-ipv6-nd-cpp --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $dir/*.proto
  protoc -I ./ --cpp_out=../src/gen-ipv6-nd-cpp $dir/*.proto
  mkdir -p ../src/gen-ipv6-nd-obj/$dir
done

for file in .; do
  protoc -I ./  --grpc_out=../src/gen-ipv6-nd-cpp --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $file/*.proto
  protoc -I ./ --cpp_out=../src/gen-ipv6-nd-cpp $file/*.proto
done

echo "Done"
