#!/bin/bash

# PATH CONFIGURATION
PROJECT_DIR="$(dirname -- "$(realpath -- "$0")")" # Current directory
# This is often the case where git reopsitories resides in a unique directory
# Else change the following directories path
# A common case is to have VTK and VT-TV in the same directory (here we named it REPOSITORIES_DIR)
REPOSITORIES_DIR="$(dirname "$PROJECT_DIR")"
VT_TV_DIR=$REPOSITORIES_DIR/vt-tv
BUILD_DIR=$REPOSITORIES_DIR/vt-tv/build
VTK_DIR=$REPOSITORIES_DIR/vtk/build

# BUILD CONFIGURATION
BUILD_TYPE=RELEASE
JOBS=10  # for instance, modify as needed
TESTS_ENABLED=ON
COVERAGE_ENABLED=OFF # Enable coverage
COVERAGE_BUILD_HTML_REPORT=OFF # Generates coverage report in HTML format
CLEAN=OFF

echo -e $'\e[32m\nVT-TV Build script\e[0m'

# # HELP FUNCTION
help() { 
  Options=$@
  cat <<EOF
  Build script for vt-tv.
  Usage: build.sh <[options]>
  Options:
          -b   --build          Set build type (DEBUG|RELEASE|COVERAGE...) ($BUILD_TYPE)
          -t   --tests          Enable tests  ($TESTS_ENABLED)
          -c   --coverage       Enable coverage  ($COVERAGE_ENABLED)
          -h   --help           Show this message
          --clean               Clean the output directory and cmake cache
EOF
  exit 1;
}

function on_off() {
  case $1 in
    TRUE) echo "ON" ;;
    true) echo "ON" ;;
    True) echo "ON" ;;
    On) echo "ON" ;;
    ON) echo "ON" ;;
    1) echo "ON" ;;
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
  case "$OPT" in
    b | build)     BUILD_TYPE=$(on_off $OPTARG) ;;
    t | tests)     TESTS_ENABLED=$(on_off $OPTARG) ;;
    c | coverage)  COVERAGE_ENABLED=$(on_off $OPTARG) ;;
    h | help )     help ;;
    clean )        CLEAN=ON ;;
    \? )           exit 2 ;;  # bad short option (error reported via getopts)
    * )            echo "Illegal option --$OPT";  exit 2 ;; # bad long option
  esac
done
shift $((OPTIND-1)) # remove parsed options and args from $@ list

set -ex

if [[ "${CLEAN}" == "ON" ]]; then
  echo "> Cleaning"
  # Remove CMakeCache for fresh build
  rm -rf CMakeCache.txt
  rm -rf ${BUILD_DIR} # recreate clean and build
fi


mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

# export CC=/usr/bin/clang
# export CXX=/usr/bin/clang++

# -DCMAKE_C_COMPILER="$(which clang)" \
# -DCMAKE_CXX_COMPILER=$(which clang++) \
# -DCMAKE_CXX_FLAGS_INIT="-std=c++17" \

# -DCMAKE_C_COMPILER="$(which clang)" \
# -DCMAKE_CXX_COMPILER=$(which clang++) \

echo "> Building (CMake|${BUILD_TYPE})..."
cmake -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DVTK_DIR=${VTK_DIR} \
  \
  -DBUILD_TESTING=ON \
  -DVT_TV_TESTS_ENABLED=ON \
  -DVT_TV_COVERAGE_ENABLED=${COVERAGE_ENABLED} \
  \
  -DVT_TV_TESTS_ALL_IN_ONE=${TESTS_ENABLED} \
  -DVT_TV_PYTHON_BINDINGS_ENABLED=OFF \
  \
  -DPython_EXECUTABLE="$(which python)" \
  -DPython_INCLUDE_DIRS=$(python -c "import sysconfig; print(sysconfig.get_path('include'))") \
  \
  "${VT_TV_DIR}"

time cmake --build . --parallel -j"${JOBS}"

# Tests and coverage
cd ${PROJECT_DIR}
if [[ "${COVERAGE_ENABLED}" == "ON" ]]; then
  echo "> Running tests (with coverage)..."
  # Tests & Coverage
  ctest --test-dir build -T Test -T Coverage || true
  #  || true to catch some strange error:
  # Error(s) while accumulating results:
  #   Problem reading source file: /home/thomas/repositories/vt-tv/lib/yaml-cpp/include/yaml-cpp/node/detail/impl.h line:235  out total: 384
  #   Looks like there are more lines in the file: /home/thomas/repositories/vt-tv/lib/yaml-cpp/include/yaml-cpp/node/detail/node.h
  lcov --capture --directory build --output-file output/lcov_vt-tv_test.info
  lcov --remove output/lcov_vt-tv_test.info -o output/lcov_vt-tv_test_no_deps.info '*/lib/*' '/usr/include/*' '*/vtk/*' '*/tests/*'
  if [[ "${COVERAGE_BUILD_HTML_REPORT}" == "ON" ]]; then
    genhtml --prefix ./src --ignore-errors source ./output/lcov_vt-tv_test_no_deps.info --legend --title "$(git rev-parse HEAD)" --output-directory=output/lcov_vt-tv_html
  else
    lcov --list output/lcov_vt-tv_test_no_deps.info
  fi
else
  echo "> Running tests..."
  # Tests only
  ctest --test-dir build -T Test
fi
