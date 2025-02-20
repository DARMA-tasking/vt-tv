#!/bin/bash

# Description: This script enable to build vt-tv
# It can also be used to run tests and coverage

set -e

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"


# A function to convert a value (1,0, Yes, no etc.) to ON or OFF.
# If no value ON will be returned
function on_off() {
  case $1 in
    TRUE|true|True|ON|on|On|1|YES|yes|Yes|Y|y) echo "ON" ;;
    FALSE|false|False|OFF|off|Off|0|NO|no|No|N|n) echo "OFF" ;;
    "") echo "ON" ;;
    *) exit 2 ;; # not supported
   esac
}

# A function to determine the number of available processors
get_num_processors() {
    case "$(uname)" in
        Linux)
            nproc
            ;;
        Darwin)
            sysctl -n hw.ncpu
            ;;
        *)
            getconf _NPROCESSORS_ONLN
            ;;
    esac
}

# Configuration
# > Build variables
# >> Path
VTK_DIR="${VTK_DIR:-$PARENT_DIR/vtk/build}"
CC="${CC:-$(which gcc || echo '')}"
CXX="${CXX:-$(which g++ || echo '')}"
GCOV="${GCOV:-gcov}"
VT_TV_DIR="${VT_TV_DIR:-$CURRENT_DIR}"
VT_TV_BUILD_DIR="${VT_TV_BUILD_DIR:-$PARENT_DIR/vt-tv/build}"
VT_TV_OUTPUT_DIR="${VT_TV_OUTPUT_DIR:-$CURRENT_DIR/output}"
VT_TV_INSTALL="${VT_TV_INSTALL:-OFF}"
VT_TV_INSTALL_DIR="${VT_TV_INSTALL_DIR:-$VT_TV_BUILD_DIR/install}"
# >> Build settings
VT_TV_BUILD=$(on_off ${VT_TV_BUILD:-ON}) # option to turn off the build to only run tests
VT_TV_BUILD_TYPE=${VT_TV_BUILD_TYPE:-Release}
VT_TV_CMAKE_JOBS=${VT_TV_CMAKE_JOBS:-$(get_num_processors)}
VT_TV_TESTS_ENABLED=$(on_off ${VT_TV_TESTS_ENABLED:-ON})
VT_TV_TEST_REPORT=${VT_TV_TEST_REPORT:-"$VT_TV_OUTPUT_DIR/junit-report.xml"}
VT_TV_COVERAGE_ENABLED=$(on_off ${VT_TV_COVERAGE_ENABLED:-OFF})
VT_TV_CLEAN=$(on_off ${VT_TV_CLEAN:-ON})
VT_TV_PYTHON_BINDINGS_ENABLED=$(on_off ${VT_TV_PYTHON_BINDINGS_ENABLED:-OFF})
VT_TV_WERROR_ENABLED=$(on_off ${VT_TV_WERROR_ENABLED:-OFF})
# >> Run tests settings
VT_TV_RUN_TESTS=$(on_off ${VT_TV_RUN_TESTS:-OFF})
VT_TV_RUN_TESTS_FILTER=${VT_TV_RUN_TESTS_FILTER:-""}
VT_TV_COVERAGE_REPORT=${VT_TV_COVERAGE_REPORT:-""}

# >> CLI args support

# # HELP FUNCTION
help() {
  cat <<EOF
  A script to build and test vt-tv.
  Options can be passed as arguments or environment variables or both (VTK_DIR, CC, CXX and VT_TV_*).

  Usage: <[environment variables]> build.sh <[options]>
  Options:
      -c   --cc=[str]               The C compiler (CC=$CC)
      -x   --cxx=[str]              The C++ compiler (CXX=$CXX)
      -k   --vtk-dir=[str]          VTK build directory (VTK_DIR=$VTK_DIR)

      -b   --build=[bool]           Build vt-tv. Can be turned off for example to run tests without rebuilding. (VT_TV_BUILD=$VT_TV_BUILD)
      -d   --build-dir=[str]        Build directory (VT_TV_BUILD_DIR=$VT_TV_BUILD_DIR)
      -i   --install                Enable installation after build (VT_TV_INSTALL=$VT_TV_INSTALL)
      -l   --install-dir=[str]      Installation directory (VT_TV_INSTALL_DIR=$VT_TV_INSTALL_DIR)
      -m   --build-type=[str]       Set the CMAKE_BUILD_TYPE value (Debug|Release|...) (VT_TV_BUILD_TYPE=$VT_TV_BUILD_TYPE)
      -y   --clean=[bool]           Clean the output directory and the CMake cache. (VT_TV_CLEAN=$VT_TV_CLEAN)
      -p   --bindings               Build with Python bindings (VT_TV_PYTHON_BINDINGS_ENABLED=$VT_TV_PYTHON_BINDINGS_ENABLED)

      -g   --coverage               Build with coverage support or enable coverage output (VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED)
      -z   --coverage-report=[str]  Target path to generate coverage HTML report files (VT_TV_COVERAGE_REPORT=$VT_TV_COVERAGE_REPORT). Empty for no report.

      -j   --jobs=[int]             Number of processors to build (VT_TV_CMAKE_JOBS=$VT_TV_CMAKE_JOBS)
      -o   --output-dir=[str]       Output directory. Used to host lcov .info files. Also default to host junit report (VT_TV_OUTPUT_DIR=$VT_TV_OUTPUT_DIR).
                                      Note: vt-tv viz output files is defined in VT-TV configuration files and might be different.
      -t   --tests=[bool]           Build vt-tv tests (VT_TV_TESTS_ENABLED=$VT_TV_TESTS_ENABLED)
      -a   --tests-report[str]      Unit tests Junit report path (VT_TV_TEST_REPORT=$VT_TV_TEST_REPORT). Empty for no report.
      -r   --tests-run=[bool]       Run unit tests (and build coverage report if coverage is enabled) (VT_TV_RUN_TESTS=$VT_TV_RUN_TESTS)
      -f   --tests-run-filter=[str] Filter unit test to run. (VT_TV_RUN_TESTS_FILTER=$VT_TV_RUN_TESTS_FILTER)

      -h   --help                   Show help and default option values.

  Examples:
      Using environment variables:
        Build & run tests:          VT_TV_RUN_TESTS=ON VT_TV_COVERAGE_ENABLED=ON build.sh
        Build with coverage:        VT_TV_TESTS_ENABLED=ON VT_TV_COVERAGE_ENABLED=ON build.sh
        Build (debug):              VT_TV_BUILD_TYPE=Debug build.sh
        Test:                       VT_TV_BUILD=OFF VT_TV_RUN_TESTS=ON build.sh
        Run Test & coverage:        VT_TV_BUILD=OFF VT_TV_COVERAGE_ENABLED=ON VT_TV_COVERAGE_REPORT=output VT_TV_RUN_TESTS=ON build.sh

      Using arguments:
        Build & Run tests:          build.sh --tests-run --coverage
        Build with coverage:        build.sh --coverage
        Build (debug):              build.sh --build-type=Debug
        Run Test & coverage:        build.sh --build=0 --tests-run --coverage --coverage-report=output/lcov-report
        Coverage report only:       build.sh --build=0 --coverage-report=output/lcov-report

      Using both:
        Build with coverage & run tests: VT_TV_COVERAGE=ON build.sh --tests-run
EOF
  exit 1;
}

while getopts btch-: OPT; do  # allow -b -t -c -h, and --long_attr=value"
  # support long options: https://stackoverflow.com/a/28466267/519360
  if [ "$OPT" == "-" ]; then   # long option: reformulate OPT and OPTARG
    OPT="${OPTARG%%=*}"       # extract long option name
    OPTARG="${OPTARG#"$OPT"}" # extract long option argument (may be empty)
    OPTARG="${OPTARG#=}"      # if long option argument, remove assigning `=`
  fi
  case "$OPT" in
    b | build )           VT_TV_BUILD=$(on_off $OPTARG) ;;
    d | build-dir )       VT_TV_BUILD_DIR=$(realpath "$OPTARG") ;;
    i | install)          VT_TV_INSTALL=$(on_off $OPTARG) ;;
    l | install-dir)      VT_TV_INSTALL_DIR=$(realpath "$OPTARG") ;;
    m | build-type)       VT_TV_BUILD_TYPE=$(on_off $OPTARG) ;;
    p | bindings )        VT_TV_PYTHON_BINDINGS_ENABLED=$(on_off $OPTARG) ;;
    c | cc)               CC="$OPTARG" ;;
    x | cxx)              CXX="$OPTARG" ;;
    y | clean)            VT_TV_CLEAN=$(on_off $OPTARG) ;;
    g | coverage)         VT_TV_COVERAGE_ENABLED=$(on_off $OPTARG) ;;
    z | coverage-report)  VT_TV_COVERAGE_REPORT=$(realpath -q "$OPTARG") ;;
    j | jobs)             VT_TV_CMAKE_JOBS=$OPTARG ;;
    o | output-dir )      VT_TV_OUTPUT_DIR=$(realpath "$OPTARG") ;;
    t | tests)            VT_TV_TESTS_ENABLED=$(on_off $OPTARG) ;;
    a | tests-report)     VT_TV_TEST_REPORT=$(realpath -q "$OPTARG") ;;
    r | tests-run )       VT_TV_RUN_TESTS=$(on_off $OPTARG) ;;
    f | tests-run-filter) VT_TV_RUN_TESTS_FILTER="$OPTARG" ;;
    k | vtk-dir )         VTK_DIR=$(realpath "$OPTARG") ;;
    h | help )            help ;;

    \? )           exit 2 ;;  # bad short option (error reported via getopts)
    * )            echo "Illegal option --$OPT";  exit 2 ;; # bad long option
  esac
done
shift $((OPTIND-1)) # remove parsed options and args from $@ list

# !! CLI args support

echo "Computed build options:"
echo VT_TV_CLEAN=$VT_TV_CLEAN
echo VT_TV_OUTPUT_DIR=$VT_TV_OUTPUT_DIR
echo VT_TV_BUILD_DIR=$VT_TV_BUILD_DIR
echo VT_TV_BUILD_TYPE=$VT_TV_BUILD_TYPE
echo VT_TV_PYTHON_BINDINGS_ENABLED=$VT_TV_PYTHON_BINDINGS_ENABLED
echo VT_TV_RUN_TESTS=$VT_TV_RUN_TESTS
echo VT_TV_TESTS_ENABLED=$VT_TV_TESTS_ENABLED
echo VT_TV_TEST_REPORT=$VT_TV_TEST_REPORT
echo VT_TV_RUN_TESTS_FILTER=$VT_TV_RUN_TESTS_FILTER
echo VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED
echo VT_TV_COVERAGE_REPORT=$VT_TV_COVERAGE_REPORT
echo CC=$CC
echo CXX=$CXX
echo GCOV=$GCOV
echo VTK_DIR=$VTK_DIR
echo DISPLAY=$DISPLAY

# Build
if [[ "${VT_TV_BUILD}" == "ON" ]]; then
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
    -DCMAKE_INSTALL_PREFIX="${VT_TV_INSTALL_DIR}" \
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

  if [[ "$VT_TV_INSTALL" == "ON" ]]; then
    echo "> Installing to ${VT_TV_INSTALL_DIR}..."
    cmake --install . --prefix "${VT_TV_INSTALL_DIR}"
  fi

  popd

fi # End build

# Run tests
if [ "$VT_TV_RUN_TESTS" == "ON" ]; then
  mkdir -p "$VT_TV_OUTPUT_DIR"
  pushd $VT_TV_OUTPUT_DIR
  # Tests
  echo "> Running tests..."
  # Run GTest unit tests and display detail for failing tests
  GTEST_OPTIONS=""
  if [ "$VT_TV_TEST_REPORT" != "" ]; then
    echo "Generating JUnit report..."
    GTEST_OPTIONS="$GTEST_OPTIONS --gtest_output=\"xml:$VT_TV_TEST_REPORT\""
  fi
  if [ "$VT_TV_RUN_TESTS_FILTER" != "" ]; then
    echo "Filtering Tests ($VT_TV_RUN_TESTS_FILTER)..."
    GTEST_OPTIONS="$GTEST_OPTIONS --gtest_filter=\"$VT_TV_RUN_TESTS_FILTER\""
  fi

  gtest_cmd="\"$VT_TV_BUILD_DIR/tests/unit/AllTests\" $GTEST_OPTIONS"
  echo "Run GTest..."
  eval "$gtest_cmd" || true
  echo "Tests done."

  popd
fi

# Coverage
if [ "$VT_TV_COVERAGE_ENABLED" == "ON" ]; then
  mkdir -p "$VT_TV_OUTPUT_DIR"
  pushd $VT_TV_OUTPUT_DIR
  # base coverage files
  echo "lcov capture:"
  lcov --capture --directory $VT_TV_BUILD_DIR --output-file lcov_vt-tv_test.info --gcov-tool $GCOV
  lcov --remove lcov_vt-tv_test.info -o lcov_vt-tv_test_no_deps.info '*/lib/*' '/usr/include/*' '*/vtk/*' '*/tests/*'
  lcov --list lcov_vt-tv_test_no_deps.info
  # optional coverage html report
  if [ "$VT_TV_COVERAGE_REPORT" != "" ]; then
    genhtml --prefix ./src --ignore-errors source lcov_vt-tv_test_no_deps.info --legend --title "$(git rev-parse HEAD)" --output-directory="$VT_TV_COVERAGE_REPORT"
  fi
  popd
fi
