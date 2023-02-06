#!/bin/bash

# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -e

[ -z "${PROTOBUF_VERSION}" ] && export PROTOBUF_VERSION="3.6.1"

cd /
wget https://github.com/google/protobuf/releases/download/v${PROTOBUF_VERSION}/protobuf-cpp-${CPP_PROTOBUF_VERSION}.tar.gz
tar zxf protobuf-cpp-${CPP_PROTOBUF_VERSION}.tar.gz --no-same-owner
cd protobuf-${CPP_PROTOBUF_VERSION}
./configure
make -j $(nproc) && make install
ldconfig
