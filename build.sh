#!/bin/bash

# Description: This script enable to build vt-tv
# It can also be used to run tests and coverage

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

# >> CLI args support

# # HELP FUNCTION
help() {
  cat <<EOF
  A script to build and test vt-tv.
  Options can be passed as arguments or environment variables or both (VTK_DIR, CC, CXX and VT_TV_*).
  Usage: <[environment variables]> build.sh <[options]>
  Options:
      -d   --build-dir=[str]      Build directory (VT_TV_BUILD_DIR=$VT_TV_BUILD_DIR)
      -b   --build-type=[str]     Set the CMAKE_BUILD_TYPE value (Debug|Release|...) (VT_TV_BUILD_TYPE=$VT_TV_BUILD_TYPE)
      -y   --clean=[ON|OFF]       Clean the output directory and the CMake cache. (VT_TV_CLEAN=$VT_TV_CLEAN)
      -c   --cc=[str]             The C compiler (CC=$CC)
      -x   --cxx=[str]            The C++ compiler (CXX=$CXX)
      -z   --coverage-html        Generates a coverage HTML report (VT_TV_COVERAGE_HTML_REPORT=$VT_TV_COVERAGE_HTML_REPORT). Require coverage enabled.
      -p   --enable-bindings      Build with Python bindings (VT_TV_PYTHON_BINDINGS_ENABLED=$VT_TV_PYTHON_BINDINGS_ENABLED)
      -g   --enable-coverage      Enable code coverage (VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED)
      -t   --enable-tests         Enable tests (VT_TV_TESTS_ENABLED=$VT_TV_TESTS_ENABLED)
      -j   --jobs=[int]           Number of processors to build (VT_TV_CMAKE_JOBS=$VT_TV_CMAKE_JOBS)
      -n   --no-build             To be used with to run tests without re-building. Combine with --run-tests=1.
      -o   --output-dir=[str]     Output directory for tests and coverage reports (VT_TV_OUTPUT_DIR=$VT_TV_OUTPUT_DIR).
                                    Note: VT-TV viz output files is defined in VT-TV configuration files and might be different.
      -r   --run-tests            Run unit tests (and build coverage report if coverage is enabled) (VT_TV_RUN_TESTS=$VT_TV_RUN_TESTS)
      -a   --tests-report[str]    Unit tests Junit report path (VT_TV_TEST_REPORT_PATH=$VT_TV_TEST_REPORT_PATH)
      -k   --vtk-dir=[str]        VTK build directory (VTK_DIR=$VTK_DIR)
      

      -h   --help                 Show help and default option values.

  Examples:
      Build & run tests:  build.sh --run-tests --enable-coverage
      Build (coverage):   build.sh --enable-coverage
      Build (debug):      build.sh --build-type=Debug
      Test:               build.sh --no-build --run-tests --coverage=1
EOF
  exit 1;
}

while getopts btch-: OPT; do  # allow -b -t -c -h, and --long_attr=value"
  # support long options: https://stackoverflow.com/a/28466267/519360
  if [ "$OPT" = "-" ]; then   # long option: reformulate OPT and OPTARG
    OPT="${OPTARG%%=*}"       # extract long option name
    OPTARG="${OPTARG#"$OPT"}" # extract long option argument (may be empty)
    OPTARG="${OPTARG#=}"      # if long option argument, remove assigning `=`
  fi
  # Note: for required ON/OFF arg with no default value:
  # $(on_off $OPTARG) || $(echo "run-tests is required" >&2; exit 2)
  case "$OPT" in
    d | build-dir )       VT_TV_BUILD_DIR=$(realpath "$OPTARG") ;;
    b | build-type)       VT_TV_BUILD_TYPE=$(on_off $OPTARG) ;;
    c | cc)               CC="$OPTARG" ;;
    x | cxx)              CXX="$OPTARG" ;;
    y | clean)            VT_TV_CLEAN=$(on_off $OPTARG) ;;
    z | coverage-html)    VT_TV_COVERAGE_HTML_REPORT=ON ;;
    p | enable-bindings ) VT_TV_PYTHON_BINDINGS_ENABLED=ON ;;
    g | enable-coverage)  VT_TV_COVERAGE_ENABLED=$(on_off $OPTARG) ;;
    t | enable-tests)     VT_TV_TESTS_ENABLED=$(on_off $OPTARG) ;;
    j | jobs)             VT_TV_CMAKE_JOBS=$OPTARG ;;
    n | no-build )        VT_TV_BUILD=OFF ;;
    o | output-dir )      VT_TV_OUTPUT_DIR=$(realpath "$OPTARG") ;;
    r | run-tests )       VT_TV_RUN_TESTS=ON ;;
    a | tests-report)     VT_TV_TEST_REPORT_PATH=$(realpath "$OPTARG") ;;
    k | vtk-dir )         VTK_DIR=$(realpath "$OPTARG") ;;
    h | help )            help ;;

    \? )           exit 2 ;;  # bad short option (error reported via getopts)
    * )            echo "Illegal option --$OPT";  exit 2 ;; # bad long option
  esac
done
shift $((OPTIND-1)) # remove parsed options and args from $@ list

# !! CLI args support


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
  # Run GTest unit tests and display detail for failing tests
  "$VT_TV_BUILD_DIR/tests/unit/AllTests" --gtest_output="xml:$VT_TV_TEST_REPORT_PATH" || true

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