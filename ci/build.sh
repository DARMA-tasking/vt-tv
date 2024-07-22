# Call root build script with specific path
# and enable tests and coverage
CMAKE_BINARY_DIR=/opt/build/vt-tv \
    VTK_DIR=/opt/build/vtk \
    VT_TV_TESTS_ENABLED=ON \
    VT_TV_COVERAGE_ENABLED=ON \
    /opt/src/vt-tv/build.sh

echo "VT-TV build success"