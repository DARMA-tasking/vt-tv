ARG BASE_IMAGE=lifflander1/vt:ubuntu_22.04-gcc_11-vtk_9.2.2-py_3.8
ARG VT_TV_TESTS_ENABLED=OFF
ARG VT_TV_COVERAGE_ENABLED=OFF
ARG VT_TV_TEST_PYTHON_BINDINGS=OFF

FROM ${BASE_IMAGE} AS base

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv

# Build
FROM base AS build
ARG VT_TV_COVERAGE_ENABLED=OFF
ARG VT_TV_TESTS_ENABLED=OFF
RUN VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED bash /opt/src/vt-tv/ci/build.sh

# Unit tests
FROM build AS test-cpp
ARG VT_TV_COVERAGE_ENABLED=OFF
ARG VT_TV_TESTS_ENABLED=OFF
RUN VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED bash /opt/src/vt-tv/ci/test.sh

# Python tests (Builds VT-TV with Python bindings & test python package)
FROM test-cpp AS test-python
# Create vizualization output directory (required)
RUN mkdir -p /opt/src/vt-tv/output/python_tests
RUN bash /opt/src/vt-tv/ci/python_build.sh
RUN bash /opt/src/vt-tv/ci/python_test.sh

# CI test script run
RUN ACTUAL=/opt/src/vt-tv/output/tests/default0.png EXPECTED=/opt/src/vt-tv/tests/expected/default/default0.png TOLERANCE=0.1 /opt/src/vt-tv/tests/test_image.sh

# Artifacts
FROM scratch AS artifacts
COPY --from=test-cpp /tmp/artifacts /tmp/artifacts
COPY --from=test-python /opt/src/vt-tv/output/python_tests /tmp/python-artifacts
