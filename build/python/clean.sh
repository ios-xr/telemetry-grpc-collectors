#!/bin/bash
#
# Copyright (c) 2016 by cisco Systems, Inc. 
# All rights reserved.
#
SCRIPT_DIR="$(cd "$(dirname "${0}")"; echo "$(pwd)")"

if [[ -d ${SCRIPT_DIR}/src/ ]];then
    rm -rf ${SCRIPT_DIR}/src
fi
