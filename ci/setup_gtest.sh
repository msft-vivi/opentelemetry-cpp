#!/bin/bash
# Ref: https://gist.github.com/kedestin/4fbf48266b2972ee1974af671250cb36
PROJECT_SOURCE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")"; cd ..; pwd )"
echo "-- PROJECT_SOURCE_DIR: $PROJECT_SOURCE_DIR"

tearup() {
    echo "-- Clear old googletest tar packages finished."
    rm -rf release*
}

# Release version may be update regularly.
# Note that different version may have different build result orgnization (this will affect mv operation).
THIRD_PARTY_ROOT=$PROJECT_SOURCE_DIR/third_party
GTEST_SOURCE_DIR=$THIRD_PARTY_ROOT/googletest
RELEASE="release-1.12.0.tar.gz"

# Backup download link
# https://github.com/google/googletest/archive/refs/tags/v1.13.0.tar.gz

echo " -- Downloading Google Test"

pushd $THIRD_PARTY_ROOT

# Clear exist tar packages
tearup

# Download googletest source code in sub shell
wget https://github.com/google/googletest/archive/${RELEASE}

if [ ! -f "$RELEASE" ]; then
    echo "Could not download GoogleTest"
    cleanup
    exit 1
fi

if [ ! -d "$GTEST_SOURCE_DIR" ]; then
    mkdir -p "$GTEST_SOURCE_DIR"
fi

echo " -- Extracting Google Test"
# Unzip to $GTEST_SOURCE_DIR path.
tar -zxvf "${RELEASE}" --strip-components=1 -C $GTEST_SOURCE_DIR

# Remove downloaded tar.
tearup
popd

GTEST_LIB_DIR=$THIRD_PARTY_ROOT/lib/googletest
if [ ! -d "$GTEST_LIB_DIR" ]; then
    echo " -- Creating $GTEST_LIB_DIR"
    mkdir -p "$GTEST_LIB_DIR"
fi

GTEST_BUILD_DIR=$GTEST_SOURCE_DIR/build
if [ -d "$GTEST_BUILD_DIR" ]; then
    rm -rf "$GTEST_BUILD_DIR"
    echo " -- Removed exist $GTEST_BUILD_DIR"
fi

build_gtest() {
    pushd $GTEST_SOURCE_DIR

    # Currently, beclow code may not excute, please check log to verify.
    mkdir build
    echo " -- Created $GTEST_BUILD_DIR"
    cd $GTEST_BUILD_DIR
    echo "pwd `pwd`"
    cmake -DCMAKE_PREFIX_PATH=$GTEST_BUILD_DIR ..
    cmake --build . --target gmock gmock_main gtest gtest_main
    # Move compiler resut to desireable path.
    mv $GTEST_BUILD_DIR/lib/lib*.a $GTEST_LIB_DIR
    echo " -- Move libgmock.a libgmock_main.a to $GTEST_LIB_DIR success"

    popd
}

build_gtest