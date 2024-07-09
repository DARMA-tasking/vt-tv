#! /bin/sh

GTEST_BRANCHTAG=release-1.12.1

# Fetch googletest from Github.
LIB_DIR="$(dirname -- "$(realpath -- "$0")")"
GTEST_DIR="${LIB_DIR}/googletest"

echo "LIB_DIR=$LIB_DIR"

mkdir -p $GTEST_DIR && rm -fr $GTEST_DIR/*
curl -L https://github.com/google/googletest/archive/refs/tags/$GTEST_BRANCHTAG.tar.gz \
   | tar zxf - -C $GTEST_DIR --strip-components 1

# Remove googletest artifacts - set cmake BUILD_GMOCK=0, don't install, don't build test.
pushd $GTEST_DIR
rm -fr .[!.]* BUILD.bazel WORKSPACE appveyor.yml library.json platformio.ini
rm -fr ci/
# rm -fr googlemock/
rm -fr googletest/test/ googletest/samples/ googletest/docs/ googletest/scripts/
rm -fr docs
popd