ARG BASE_IMAGE=pierrpebay/vt-tv:ubuntu_22.04-gcc_11-vtk_9.2.2-py_3.8

FROM ${BASE_IMAGE} AS base

# setup requirements for rendering tests (xvfb) + coverage report (lcov)
RUN apt-get update && apt-get install -y \
    xvfb \
    lcov

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv

# Load variables to bash (CC, CXX, VTK_DIR)
VOLUME /vol1
RUN /bin/bash -c "set -a && source /vol1/.env && set +a"

# Build
FROM base AS build
RUN chmod +x /opt/src/vt-tv/build.sh
RUN CMAKE_BINARY_DIR=/opt/build/vt-tv \
    CC="$BUILD_CONFIG[2]-$BUILD_CONFIG[3]" \
    CXX="$BUILD_CONFIG[4]${CXX}" \
    VTK_DIR=/opt/build/vtk \
    VTK_DIR=/opt/build/vtk \
    VT_TV_TESTS_ENABLED=ON \
    VT_TV_COVERAGE_ENABLED=ON \
    /opt/src/vt-tv/build.sh

RUN echo "VT-TV build success"

# Unit tests
FROM build AS test-cpp
RUN ["chmod", "+x", "/opt/src/vt-tv/ci/test_cpp.sh"]
RUN "/opt/src/vt-tv/ci/test_cpp.sh"
RUN bash /opt/src/vt-tv/ci/docker/test.sh

# Unit tests
FROM build AS test-cpp
RUN ["chmod", "+x", "/opt/src/vt-tv/ci/test_cpp.sh"]
RUN "/opt/src/vt-tv/ci/test_cpp.sh"
RUN bash /opt/src/vt-tv/ci/docker/test.sh

# Python tests (Builds VT-TV with Python bindings & test python package)
FROM build AS test-python
RUN ["chmod", "+x", "/opt/src/vt-tv/ci/test_python.sh"]
RUN "/opt/src/vt-tv/ci/test_python.sh"

# Artifacts
FROM scratch AS artifacts
COPY --from=test-python /tmp/artifacts /tmp/artifacts
COPY --from=test-cpp /tmp/artifacts /tmp/artifacts
