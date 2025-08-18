#!/bin/bash

# This script builds vt-tv for CI including tests and coverage

set -ex

source_dir=${1}
build_dir=${2}

export VT_TV=${source_dir}
export VT_TV_BUILD=${build_dir}/vt-tv

if hash ccache &>/dev/null
then
    use_ccache=true
fi

if test "$use_ccache"
then
    { echo -e "===\n=== ccache statistics before build\n==="; } 2>/dev/null
    ccache -s
else
    { echo -e "===\n=== ccache not found, compiling without it\n==="; } 2>/dev/null
fi

mkdir -p "$VT_TV_BUILD"
cd "$VT_TV_BUILD"
rm -Rf ./*

cmake -B "${VT_TV_BUILD}" \
   -DCMAKE_BUILD_TYPE:STRING="${VT_TV_BUILD_TYPE:-Release}" \
   -DVTK_DIR="${VTK_DIR:-/opt/vtk}" \
   -DCMAKE_C_COMPILER="${CC:-gcc}" \
   -DCMAKE_CXX_COMPILER="${CXX:-g++}" \
   -DCMAKE_INSTALL_PREFIX="${VT_TV_INSTALL_DIR:-${VT_TV_BUILD}/install}" \
   -DVT_TV_WERROR_ENABLED="${VT_TV_WERROR_ENABLED:-ON}" \
   -DVT_TV_TESTS_ENABLED="${VT_TV_TESTS_ENABLED:-ON}" \
   -DVT_TV_COVERAGE_ENABLED="${VT_TV_COVERAGE_ENABLED:-OFF}" \
   -DVT_TV_PYTHON_BINDINGS_ENABLED="${VT_TV_PYTHON_BINDINGS_ENABLED:-OFF}" \
   -DPython_EXECUTABLE="$(which python)" \
   -DPython_INCLUDE_DIRS="$(python -c "import sysconfig; print(sysconfig.get_path('include'))")" \
   "${VT_TV}"

time cmake --build . --target install

if test "$use_ccache"
then
    { echo -e "===\n=== ccache statistics after build\n==="; } 2>/dev/null
    ccache -s
fi
