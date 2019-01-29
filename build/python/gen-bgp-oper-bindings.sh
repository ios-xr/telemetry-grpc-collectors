#!/bin/bash
#
# Copyright (c) 2016 by cisco Systems, Inc. 
# All rights reserved.
#
SCRIPT_DIR="$(cd "$(dirname "${0}")"; echo "$(pwd)")"

declare -a build_paths=("cisco_ios_xr_ipv4_bgp_oper" "mdt_grpc_dialout" "mdt_grpc_dialin")

containsPath () {
  local match="$1"
  match_dir=$(echo "$1" | cut -d "/" -f2)
  for element in "${build_paths[@]}"
  do 
      if [[ $element == $match_dir ]];then
          return 0 
      fi    
  done
  return 1
}

# Removing old bindings
if [ -d ${SCRIPT_DIR}/src/genpy-bgp-oper ]; then
  rm -rf ${SCRIPT_DIR}/src/genpy-bgp-oper
fi

printf "Generating Python bindings..."
mkdir -p ${SCRIPT_DIR}/src/genpy-bgp-oper

cd ../../bigmuddy-network-telemetry-proto/proto_archive/

for dir in `find . -type d -links 2`; do
  containsPath $dir
  if [[ $? == 0 ]]; then
      python -m grpc_tools.protoc -I ./ --python_out=${SCRIPT_DIR}/src/genpy-bgp-oper/ --grpc_python_out=${SCRIPT_DIR}/src/genpy-bgp-oper/ $dir/*.proto
      mkdir -p ${SCRIPT_DIR}/src/genpy-bgp-oper/$dir
      touch ${SCRIPT_DIR}/src/genpy-bgp-oper/${dir}/__init__.py
      2to3 -w ${SCRIPT_DIR}/src/genpy-bgp-oper/${dir}/*.py >/dev/null 2>&1 
  fi
done

for file in .; do
  python -m grpc_tools.protoc -I ./ --python_out=${SCRIPT_DIR}/src/genpy-bgp-oper/ --grpc_python_out=${SCRIPT_DIR}/src/genpy-bgp-oper/ *.proto
  touch ${SCRIPT_DIR}/src/genpy-bgp-oper/__init__.py
  2to3 -w ${SCRIPT_DIR}/src/genpy-bgp-oper/*.py >/dev/null 2>&1 
done

echo "Done"
