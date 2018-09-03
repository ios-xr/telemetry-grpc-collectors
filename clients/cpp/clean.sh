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

set -x

SCRIPT_DIR="$(cd "$(dirname "${0}")"; echo "$(pwd)")"

cd ${SCRIPT_DIR}
make clean

${SCRIPT_DIR}/../../build/cpp/clean_ipv6_nd_lib.sh
