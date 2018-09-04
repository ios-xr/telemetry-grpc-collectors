#!/bin/bash
#
# Copyright (c) 2014-present, Facebook, Inc.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.
#

if [[ $(/usr/bin/id -u) -ne 0 ]]; then
    echo "Not running as sudo"
    echo "Please run the script as : sudo <scriptpath>"
    exit
fi

SCRIPT_DIR="$(cd "$(dirname "${0}")"; echo "$(pwd)")"

${SCRIPT_DIR}/../../build/cpp/build_ipv6_nd_lib.sh -p 3.5.0 -g 1.7.0

BUILD_DIR="$(readlink -f "$(dirname "$0")")"
export DESTDIR=""
mkdir -p "$BUILD_DIR/deps"
cd "$BUILD_DIR/deps"

find_github_hash() {
  if [[ $# -eq 1 ]]; then
    rev_file="github_hashes/$1-rev.txt"
    if [[ -f "$rev_file" ]]; then
      head -1 "$rev_file" | awk '{ print $3 }'
    fi
  fi
}

install_glog() {
  glog_installed=`pkg-config --exists libglog && echo exists`
  if [[ $glog_installed != "exists" ]]; then
    pushd .
    if [[ ! -e "glog" ]]; then
      git clone https://github.com/google/glog
    fi
    cd glog
    git fetch origin
    git checkout v0.3.5
    set -eu && autoreconf -i
    ./configure
    make
    make install
    ldconfig
    popd
  fi
}

#
# Install required tools and libraries via package managers
#

apt-get install -y libdouble-conversion-dev \
  libssl-dev \
  cmake \
  make \
  zip \
  git \
  autoconf \
  autoconf-archive \
  automake \
  libtool \
  g++ \
  libboost-all-dev \
  libevent-dev \
  flex \
  bison \
  liblz4-dev \
  liblzma-dev \
  scons \
  libkrb5-dev \
  libsnappy-dev \
  libsasl2-dev \
  libnuma-dev \
  pkg-config \
  zlib1g-dev \
  binutils-dev \
  libjemalloc-dev \
  libiberty-dev \
  python-setuptools \
  python3-setuptools \
  python-pip

#
# install other dependencies from source
#

install_glog

rm -rf ${SCRIPT_DIR}/deps

cd ${SCRIPT_DIR}/
make

exit 0
