# Calls build script with CI configuration
# (Specific path, enable tests and coverage)
CMAKE_BINARY_DIR=/opt/build/vt-tv
    VTK_DIR=/opt/build/vtk
    VT_TV_TESTS_ENABLED=ON
    VT_TV_COVERAGE_ENABLED=ON
    /opt/src/vt-tv/build.sh
    #/home/thomas/repositories/vt-tv/build.sh # local test

echo "VT-TV build success"