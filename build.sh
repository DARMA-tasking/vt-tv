#!/bin/bash

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")" # Current directory
PARENT_DIR="$(dirname "$CURRENT_DIR")"

# A function to convert a value to ON or OFF
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
VT_TV_BUILD_TYPE=${VT_TV_BUILD_TYPE:-Release}
VT_TV_CMAKE_JOBS=${VT_TV_CMAKE_JOBS:-$(nproc)}
VT_TV_TESTS_ENABLED=$(on_off ${VT_TV_TESTS_ENABLED:-ON})
VT_TV_COVERAGE_ENABLED=$(on_off ${VT_TV_COVERAGE_ENABLED:-OFF})
VT_TV_COVERAGE_REPORT=$(on_off ${VT_TV_COVERAGE_REPORT:-ON})
VT_TV_CLEAN=$(on_off ${VT_TV_CLEAN:-ON})
VT_TV_PYTHON_BINDINGS_ENABLED=$(on_off ${VT_TV_PYTHON_BINDINGS_ENABLED:-OFF})
# >> Run tests settings
VT_TV_RUN_TESTS=$(on_off ${VT_TV_RUN_TESTS:-OFF})
VT_TV_RUN_TESTS_ONLY=$(on_off ${VT_TV_RUN_TESTS_ONLY:-OFF})

echo "VT_TV_RUN_TESTS_ONLY="$VT_TV_RUN_TESTS_ONLY

if [[ "${VT_TV_RUN_TESTS_ONLY}" == "OFF" ]]; then
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
    -DCMAKE_CXX_FLAGS="-Werror" \
    \
    -DBUILD_TESTING=${VT_TV_TESTS_ENABLED} \
    -DVT_TV_TESTS_ENABLED=${VT_TV_TESTS_ENABLED} \
    -DVT_TV_COVERAGE_ENABLED=${VT_TV_COVERAGE_ENABLED} \
    \
    -DVT_TV_TESTS_ALL_IN_ONE=${VT_TV_TESTS_ENABLED} \
    -DVT_TV_PYTHON_BINDINGS_ENABLED=${VT_TV_PYTHON_BINDINGS_ENABLED} \
    \
    -DPython_EXECUTABLE="$(which python)" \
    -DPython_INCLUDE_DIRS=$(python -c "import sysconfig; print(sysconfig.get_path('include'))") \
    \
    "${VT_TV_DIR}"

  time cmake --build . --parallel -j "${VT_TV_CMAKE_JOBS}"

  popd

fi # End build

if [[ "${VT_TV_RUN_TESTS}" == "ON" ]]; then
  pushd ${VT_TV_OUTPUT_DIR}
  # Tests and coverage
  if [[ "${VT_TV_COVERAGE_ENABLED}" == "ON" ]]; then
    echo "> Running tests (with coverage)..."
    # Tests & Coverage
    ctest --test-dir ${BUILD_DIR} -T Test -T Coverage || true
    #  || true to continue on error:
    # Encoutered error with coverageis:
    # Error(s) while accumulating results:
    #   Problem reading source file: /home/thomas/repositories/vt-tv/lib/yaml-cpp/include/yaml-cpp/node/detail/impl.h line:235  out total: 384
    #   Looks like there are more lines in the file: /home/thomas/repositories/vt-tv/lib/yaml-cpp/include/yaml-cpp/node/detail/node.h
    
    lcov --capture --directory ${VT_TV_BUILD_DIR} --output-file lcov_vt-tv_test.info
    lcov --remove lcov_vt-tv_test.info -o lcov_vt-tv_test_no_deps.info '*/lib/*' '/usr/include/*' '*/vtk/*' '*/tests/*'
    lcov --summary lcov_vt-tv_test_no_deps.info
    if [[ "${VT_TV_COVERAGE_REPORT}" == "ON" ]]; then
      genhtml --prefix ./src --ignore-errors source lcov_vt-tv_test_no_deps.info --legend --title "$(git rev-parse HEAD)" --output-directory=lcov_vt-tv_html
    else
      lcov --list lcov_vt-tv_test_no_deps.info
    fi
  else
    echo "> Running tests..."
    # Tests only
    ctest --test-dir ${BUILD_DIR} -T Test || true
  fi
  popd
fi