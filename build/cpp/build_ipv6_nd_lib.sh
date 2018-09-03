#!/bin/bash

SCRIPT_PATH=$(dirname `which $0`)

cd $SCRIPT_PATH

mkdir -p proto/

# Extract proto files associated with IPv6 ND Oper data

proto_archive_ipv6_nd="../../bigmuddy-network-telemetry-proto/proto_archive/cisco_ios_xr_ipv6_nd_oper/"

cp -r ${proto_archive_ipv6_nd}/cisco_ios_xr_ipv6_nd_oper/  ./proto/cisco_ios_xr_ipv6_nd_oper/ 
cp -r ${proto_archive_ipv6_nd}/mdt_grpc_dialin/  ./proto/mdt_grpc_dialin
cp -r ${proto_archive_ipv6_nd}/mdt_grpc_dialout/ ./proto/mdt_grpc_dialout
cp ${proto_archive_ipv6_nd}/telemetry.proto ./proto/


#Generate the c++ binding from proto files

./gen-cpp-bindings-ipv6-nd.sh

# Compile the object files and build library
make
