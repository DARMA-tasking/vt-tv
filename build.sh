#!/bin/bash

# Old script for CI only was
# set -ex

# cmake -DCMAKE_BUILD_TYPE=Release \
#     -DCMAKE_C_COMPILER=gcc-11 \
#     -DCMAKE_CXX_COMPILER=g++-11 \
#     -DCMAKE_CXX_FLAGS="-Werror" \
#     -DVTK_DIR=/opt/build/vtk-build \
#     -S /opt/src/vt-tv -B /opt/build/vt-tv

# time cmake --build /opt/build/vt-tv --parallel $(nproc)


# PATH CONFIGURATION
PROJECT_DIR="$(dirname -- "$(realpath -- "$0")")" # Current directory
# This is often the case where git reopsitories resides in a unique directory
# Else change the following directories path
# A common case is to have VTK and VT-TV in the same directory (here we named it REPOSITORIES_DIR)
REPOSITORIES_DIR="$(dirname "$PROJECT_DIR")"
VT_TV_DIR=$REPOSITORIES_DIR/vt-tv
BUILD_DIR=$REPOSITORIES_DIR/vt-tv/build
VTK_DIR=$REPOSITORIES_DIR/vtk/build
OUTPUT_DIR=$REPOSITORIES_DIR/vt-tv/output

# BUILD CONFIGURATION
BUILD_TYPE=RELEASE # default build RELEASE
JOBS=$(nproc)  # default number of processors
TESTS_ENABLED=ON # default enable tests
COVERAGE_ENABLED=OFF # Enable coverage
COVERAGE_BUILD_HTML_REPORT=ON # Generates coverage report in HTML format if COVERAGE_ENABLED is ON
CLEAN=ON
PYTHON_BINDINGS_ENABLED=OFF
C_COMPILER="$(which gcc)"
CXX_COMPILER="$(which g++)"
RUN_TESTS=OFF
DO_BUILD=ON

set -e

echo -e $'\e[32m\nVT-TV Build script\e[0m'

# # HELP FUNCTION
help() {
  cat <<EOF
  Highly configurable script to build vt-tv either for local build or CI build.
  Usage: build.sh <[options]>
  Options:
          -b   --build          Set build type (DEBUG|RELEASE) ($BUILD_TYPE)
          -t   --tests          Enable tests  ($TESTS_ENABLED)
          -c   --coverage       Enable code coverage  ($COVERAGE_ENABLED)
          -p   --procs          Number of processors to build ($JOBS)
               --python         Build with Python bindings ($PYTHON_BINDINGS_ENABLED)
               --clean          Clean the output directory and the CMake cache
               --cc             The C compiler ($C_COMPILER)
               --cxxc           The C++ compiler ($CXX_COMPILER)
               --no-build       To be used with to run tests without re-building. Combine with --run-tests=1.
               --run-tests      Run unit tests (and optionally coverage) ($RUN_TESTS)
               --vtk-dir        VTK build directory ($VTK_DIR)
               --build-dir      Build directory ($BUILD_DIR)
               --output-dir     Output directory for the coverage HTML report ($OUTPUT_DIR).
                                Note: VT-TV output directory is defined in VT-TV configuration file.
                                      A relative path is resolved from the project directory.
                                      This base output directory is not currently configurable for vt-tv output directory.
          -h   --help           Show this help text

  Examples:
          build.sh --tests=1 --coverage=1 --build=RELEASE --clean --run-tests
          build.sh --no-build --run-tests=1 --coverage=1
EOF
  exit 1;
}

function on_off() {
  default=$2
  if [[ "${default}" == "" ]]; then
    default="ON"
  fi
  case $1 in
    TRUE) echo "ON" ;;
    true) echo "ON" ;;
    True) echo "ON" ;;
    On) echo "ON" ;;
    ON) echo "ON" ;;
    1) echo "ON" ;;
    "") exit 2 ;;
    *) echo "OFF" ;;
   esac
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
    b | build)     BUILD_TYPE=$(on_off $OPTARG) ;;
    t | tests)     TESTS_ENABLED=$(on_off $OPTARG) ;;
    c | coverage)  COVERAGE_ENABLED=$(on_off $OPTARG) ;;
    p | procs)     JOBS=$OPTARG ;;
    cc)            C_COMPILER="$OPTARG" ;;
    cxxc)          CXX_COMPILER="$OPTARG" ;;
    h | help )     help ;;
    python )       PYTHON_BINDINGS_ENABLED=ON ;;
    run-tests )    RUN_TESTS=$(on_off $OPTARG ON) ;;
    no-build )     DO_BUILD=OFF ;;
    vtk-dir )      VTK_DIR=$(realpath "$OPTARG") ;;
    build-dir )    BUILD_DIR=$(realpath "$OPTARG") ;;
    output-dir )   OUTPUT_DIR=$(realpath "$OPTARG") ;;
    clean )        CLEAN=ON ;;
    \? )           exit 2 ;;  # bad short option (error reported via getopts)
    * )            echo "Illegal option --$OPT";  exit 2 ;; # bad long option
  esac
done
shift $((OPTIND-1)) # remove parsed options and args from $@ list

# set -ex
# echo "RUN_TESTS=$RUN_TESTS"
# exit 0

if [[ "${DO_BUILD}" == "ON" ]]; then

  if [[ "${CLEAN}" == "ON" ]]; then
    echo "> Cleaning"
    # Remove CMakeCache for fresh build
    rm -rf CMakeCache.txt
    rm -rf ${BUILD_DIR} # recreate clean and build
  fi

  mkdir -p ${BUILD_DIR}
  pushd ${BUILD_DIR}

  echo "> Building (CMake|${BUILD_TYPE})..."
  cmake -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DVTK_DIR=${VTK_DIR} \
    \
    -DCMAKE_C_COMPILER="${C_COMPILER}" \
    -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
    \
    -DCMAKE_CXX_FLAGS="-Werror" \
    \
    -DBUILD_TESTING=${TESTS_ENABLED} \
    -DVT_TV_TESTS_ENABLED=${TESTS_ENABLED} \
    -DVT_TV_COVERAGE_ENABLED=${COVERAGE_ENABLED} \
    \
    -DVT_TV_TESTS_ALL_IN_ONE=${TESTS_ENABLED} \
    -DVT_TV_PYTHON_BINDINGS_ENABLED=${PYTHON_BINDINGS_ENABLED} \
    \
    -DPython_EXECUTABLE="$(which python)" \
    -DPython_INCLUDE_DIRS=$(python -c "import sysconfig; print(sysconfig.get_path('include'))") \
    \
    "${VT_TV_DIR}"

  time cmake --build . --parallel -j "${JOBS}"

  popd

fi # End build

if [[ "${RUN_TESTS}" == "ON" ]]; then

  # Tests and coverage
  pushd ${PROJECT_DIR}
  if [[ "${COVERAGE_ENABLED}" == "ON" ]]; then
    echo "> Running tests (with coverage)..."
    # Tests & Coverage
    ctest --test-dir ${BUILD_DIR} -T Test -T Coverage || true
    #  || true to continue on error:
    # Encoutered error with coverageis:
    # Error(s) while accumulating results:
    #   Problem reading source file: /home/thomas/repositories/vt-tv/lib/yaml-cpp/include/yaml-cpp/node/detail/impl.h line:235  out total: 384
    #   Looks like there are more lines in the file: /home/thomas/repositories/vt-tv/lib/yaml-cpp/include/yaml-cpp/node/detail/node.h
    pushd OUTPUT_DIR
    lcov --capture --directory ${BUILD_DIR} --output-file lcov_vt-tv_test.info
    lcov --remove lcov_vt-tv_test.info -o lcov_vt-tv_test_no_deps.info '*/lib/*' '/usr/include/*' '*/vtk/*' '*/tests/*'
    if [[ "${COVERAGE_BUILD_HTML_REPORT}" == "ON" ]]; then
      genhtml --prefix ./src --ignore-errors source lcov_vt-tv_test_no_deps.info --legend --title "$(git rev-parse HEAD)" --output-directory=lcov_vt-tv_html
    else
      lcov --list lcov_vt-tv_test_no_deps.info
    fi
    popd
  else
    echo "> Running tests..."
    # Tests only
    ctest --test-dir ${BUILD_DIR} -T Test
  fi

  popd
fi