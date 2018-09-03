#!/bin/bash

usage="
$(basename "$0") [-h] [-g/--grpc-version -p/--protobuf-version -v/--verbose] -- script to install desired versions of grpc, protobuf and build the libxrtelemetry.a library 
where:
    -h  show this help text
    -g/--grpc-version specify the grpc version to be installed (mandatory argument) 
    -p/--protobuf-version specify the protobuf version to be installed (mandatory argument)
    -v  get more verbose information during script execution
"

while true; do
  case "$1" in
    -v | --verbose )     VERBOSE=true; shift ;;
    -h | --help )        echo "$usage"; exit 0 ;;
    -g | --grpc-version )   GRPC_VERSION=$2; shift; shift;;
    -p | --protobuf-version ) PROTOBUF_VERSION=$2; shift; shift;; 
    -- ) shift; break ;;
    * ) break ;;
  esac
done

if ! [[ $GRPC_VERSION ]] || ! [[ $PROTOBUF_VERSION ]]; then
   echo "Must specify both  -g/--grpc--version and -p/--protobuf-version, see usage below"
   echo "$usage"
   exit 0
fi

if [[ $VERBOSE ]];then
    set -x
fi

# Install pkg-config first
apt-get update && apt-get install -y \
         autoconf automake libtool curl make g++ unzip git pkg-config

PROTOBUF_INSTALLED_VERSION=`pkg-config --exists protobuf && pkg-config --modversion protobuf`
GRPC_INSTALLED_VERSION=`pkg-config --exists grpc && pkg-config --modversion grpc++`


SCRIPT_DIR="$(cd "$(dirname "${0}")"; echo "$(pwd)")"



if [[ $GRPC_INSTALLED_VERSION != $GRPC_VERSION ]] ||
        [[ $PROTOBUF_INSTALLED_VERSION != $PROTOBUF_VERSION ]]; then 

    rm -rf ~/tempdir
    mkdir -p ~/tempdir/protobuf

    if [[ $PROTOBUF_INSTALLED_VERSION != $PROTOBUF_VERSION ]]; then
        #install protobuf
        cd ~/tempdir/protobuf
        curl -OL https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOBUF_VERSION}/protobuf-all-${PROTOBUF_VERSION}.tar.gz && \
        tar -zxvf protobuf-all-${PROTOBUF_VERSION}.tar.gz && \
        ./configure && \
        make && \
        make install &&\
        ldconfig 

    fi

    if [[ $GRPC_INSTALLED_VERSION != $GRPC_VERSION ]]; then
        #install grpc
        git clone https://github.com/grpc/grpc.git -b v${GRPC_VERSION} ~/tempdir/grpc && \
        cd ~/tempdir/grpc && \
        git submodule update --init && \
        make && \
        make install 
    fi
fi

cd ~/ && rm -rf ~/tempdir

SCRIPT_PATH=$(dirname `which $0`)

cd $SCRIPT_PATH

# Clean up first
$SCRIPT_PATH/clean_ipv6_nd_lib.sh

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
