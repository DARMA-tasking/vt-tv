#!/usr/bin/env bash

set -exo pipefail

source_dir=${1}
build_dir=${2}

export VT_TV=${source_dir}
export VT_TV_BUILD=${build_dir}/vt-tv

export VT_TV_OUTPUT_DIR="${VT_TV_OUTPUT_DIR:-$source_dir/output}"

pushd "$VT_TV_BUILD"

# Active conda env that contains needed python packages
eval "$("$CONDA_PATH/bin/conda" shell.bash hook)"
conda activate py3.12

#Run tests
mkdir -p "$VT_TV_OUTPUT_DIR"
# Tests
echo "> Running tests..."
# Run GTest unit tests and display detail for failing tests
GTEST_OPTIONS=""

if [ "$VT_TV_TEST_REPORT" != "" ]; then
  echo "Generating JUnit report..."
  GTEST_OPTIONS="$GTEST_OPTIONS --gtest_output=\"xml:$VT_TV_OUTPUT_DIR/$VT_TV_TEST_REPORT\""
fi

if [ "$VT_TV_RUN_TESTS_FILTER" != "" ]; then
  echo "Filtering Tests ($VT_TV_RUN_TESTS_FILTER)..."
  GTEST_OPTIONS="$GTEST_OPTIONS --gtest_filter=\"$VT_TV_RUN_TESTS_FILTER\""
fi

gtest_cmd="\"$VT_TV_BUILD/tests/unit/AllTests\" $GTEST_OPTIONS"
echo "Run GTest..."
eval "$gtest_cmd" || true
echo "Tests done."

if [ "$VT_TV_COVERAGE_ENABLED" == "ON" ]; then
  lcov --gcov-tool /usr/bin/gcov-12 --directory "$VT_TV_BUILD" --capture --output-file coverage.info
  lcov --remove coverage.info '/usr/*' '/opt/vtk/*' --output-file coverage.info
  lcov --list coverage.info
fi


# Add artifacts
VT_TV_ARTIFACTS_DIR="/tmp/artifacts"
mkdir -p $VT_TV_ARTIFACTS_DIR

# > go to output directory
pushd "$VT_TV_OUTPUT_DIR"

# > add the unit tests report artifact
cp "junit-report.xml" $VT_TV_ARTIFACTS_DIR/ || true

# > add mesh files and png artifacts
if [ -d "$VT_TV_TESTS_OUTPUT_DIR" ]; then
    cp "$VT_TV_TESTS_OUTPUT_DIR/"*".vtp"  $VT_TV_ARTIFACTS_DIR/
    cp "$VT_TV_TESTS_OUTPUT_DIR/"*".png"  $VT_TV_ARTIFACTS_DIR/
fi

popd

# list artifacts dir content
ls $VT_TV_ARTIFACTS_DIR
