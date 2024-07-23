#! /bin/sh

YAML_CPP_TAG=yaml-cpp-0.7.0 # next is 0.8.0

# Fetch googletest from Github.
LIB_DIR="$(dirname -- "$(realpath -- "$0")")"
YAML_CPP_DIR="${LIB_DIR}/yaml-cpp"

echo "LIB_DIR=$LIB_DIR"

mkdir -p $YAML_CPP_DIR && rm -fr $YAML_CPP_DIR/*
# curl -L https://github.com/jbeder/yaml-cpp/archive/refs/tags/$YAML_CPP_TAG.tar.gz \
#    | tar zxf - -C $YAML_CPP_DIR --strip-components 1
curl -L https://github.com/jbeder/yaml-cpp/archive/refs/heads/master.tar.gz \
   | tar zxf - -C $YAML_CPP_DIR --strip-components 1

# Remove googletest artifacts - set cmake BUILD_GMOCK=0, don't install, don't build test.
# pushd $GTEST_DIR
# rm -fr .[!.]* BUILD.bazel WORKSPACE appveyor.yml library.json platformio.ini
# rm -fr ci/
# rm -fr googletest/test/ googletest/samples/ googletest/docs/ googletest/scripts/
# rm -fr googlemock/test/ googlemock/samples/ googlemock/docs/ googlemock/scripts/
# rm -fr docs
# popd