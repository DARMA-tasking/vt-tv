# Calls build script with CI configuration
# (Specific path, enable tests and coverage)
CMAKE_BINARY_DIR=/opt/build/vt-tv
    VTK_DIR=/opt/build/vtk
    VT_TV_TESTS_ENABLED=ON
    VT_TV_COVERAGE_ENABLED=ON
    /home/thomas/repositories/vt-tv/build.sh
    #/opt/src/vt-tv/build.sh
    

echo "VT-TV build success"