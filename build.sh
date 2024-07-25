#!/bin/bash

# Description: This script enable to build vt-tv and/or run vt-tv unit tests

set -e

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")" # Current directory
PARENT_DIR="$(dirname "$CURRENT_DIR")"

# A function to convert a value (1,0, Yes, no etc.) to ON or OFF
function on_off() {
  case $1 in
    TRUE|true|True|ON|on|On|1|YES|yes|Yes|Y|y) echo "ON" ;;
    FALSE|false|False|OFF|off|Off|0|NO|no|No|N|n) echo "OFF" ;;
    "") exit 2 ;;
    *) echo "OFF" ;;
   esac
}

# Configuration
# > Build variables
# >> Path
VTK_DIR="${VTK_DIR:-$PARENT_DIR/vtk/build}"
CC="${CC:-$(which gcc || echo '')}"
CXX="${CXX:-$(which g++ || echo '')}"
VT_TV_DIR="${VT_TV_DIR:-$CURRENT_DIR}"
VT_TV_BUILD_DIR="${VT_TV_BUILD_DIR:-$PARENT_DIR/vt-tv/build}"
VT_TV_OUTPUT_DIR="${VT_TV_OUTPUT_DIR:-$CURRENT_DIR/output}"
# >> Build settings
VT_TV_BUILD=$(on_off ${VT_TV_NO_BUILD:-ON}) # option to turn off the build to only run tests
VT_TV_BUILD_TYPE=${VT_TV_BUILD_TYPE:-Release}
VT_TV_CMAKE_JOBS=${VT_TV_CMAKE_JOBS:-$(nproc)}
VT_TV_TESTS_ENABLED=$(on_off ${VT_TV_TESTS_ENABLED:-ON})
VT_TV_TEST_REPORT_PATH=${VT_TV_TEST_REPORT_PATH:-"$VT_TV_OUTPUT_DIR/junit-report.xml"}
VT_TV_COVERAGE_ENABLED=$(on_off ${VT_TV_COVERAGE_ENABLED:-OFF})
VT_TV_CLEAN=$(on_off ${VT_TV_CLEAN:-ON})
VT_TV_PYTHON_BINDINGS_ENABLED=$(on_off ${VT_TV_PYTHON_BINDINGS_ENABLED:-OFF})
VT_TV_WERROR_ENABLED=$(on_off ${VT_TV_WERROR_ENABLED:-OFF})
# >> Run tests settings
VT_TV_RUN_TESTS=$(on_off ${VT_TV_RUN_TESTS:-OFF})

VT_TV_COVERAGE_HTML_REPORT=$(on_off ${VT_TV_COVERAGE_HTML_REPORT:-OFF})

# Build
if [[ "${VT_TV_BUILD}" == "ON" ]]; then
  echo la
  if [[ "${VT_TV_CLEAN}" == "ON" ]]; then
    echo "> Cleaning"
    # Remove CMakeCache for fresh build
    rm -rf CMakeCache.txt
    rm -rf ${VT_TV_BUILD_DIR} # recreate clean and build
  fi

  mkdir -p ${VT_TV_BUILD_DIR}
  pushd ${VT_TV_BUILD_DIR}

  echo "> Building (CMake|${VT_TV_BUILD_TYPE})..."
  cmake -B "${VT_TV_BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE:STRING=${VT_TV_BUILD_TYPE} \
    -DVTK_DIR=${VTK_DIR} \
    \
    -DCMAKE_C_COMPILER="${CC}" \
    -DCMAKE_CXX_COMPILER="${CXX}" \
    \
    -DVT_TV_WERROR_ENABLED="${VT_TV_WERROR_ENABLED}" \
    \
    -DBUILD_TESTING=${VT_TV_TESTS_ENABLED} \
    -DVT_TV_TESTS_ENABLED=${VT_TV_TESTS_ENABLED} \
    -DVT_TV_COVERAGE_ENABLED=${VT_TV_COVERAGE_ENABLED} \
    \
    -DVT_TV_PYTHON_BINDINGS_ENABLED=${VT_TV_PYTHON_BINDINGS_ENABLED} \
    \
    -DPython_EXECUTABLE="$(which python)" \
    -DPython_INCLUDE_DIRS=$(python -c "import sysconfig; print(sysconfig.get_path('include'))") \
    \
    "${VT_TV_DIR}"

  time cmake --build . --parallel -j "${VT_TV_CMAKE_JOBS}"

  popd

fi # End build

# Run tests
if [[ "$VT_TV_RUN_TESTS" == "ON" ]]; then

  mkdir -p "$VT_TV_OUTPUT_DIR"

  pushd $VT_TV_OUTPUT_DIR
  # Tests
  echo "> Running tests (coverage $VT_TV_COVERAGE_ENABLED)..."
  # run with gtest and 
  "$VT_TV_BUILD_DIR/tests/unit/AllTests" --gtest_brief=1 --gtest_output="xml:$VT_TV_TEST_REPORT_PATH" || true

  # Coverage reports
  if [[ "$VT_TV_COVERAGE_ENABLED" == "ON" ]]; then
    lcov --capture --directory $VT_TV_BUILD_DIR --output-file lcov_vt-tv_test.info
    lcov --remove lcov_vt-tv_test.info -o lcov_vt-tv_test_no_deps.info '*/lib/*' '/usr/include/*' '*/vtk/*' '*/tests/*'
    lcov --summary lcov_vt-tv_test_no_deps.info
    lcov --list lcov_vt-tv_test_no_deps.info
    if [[ "$VT_TV_COVERAGE_HTML_REPORT" == "ON" ]]; then
      genhtml --prefix ./src --ignore-errors source lcov_vt-tv_test_no_deps.info --legend --title "$(git rev-parse HEAD)" --output-directory=lcov_vt-tv_html
    fi
  fi
  popd
fi