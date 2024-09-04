ARG BASE_IMAGE=lifflander1/vt:ubuntu_22.04-gcc_11-vtk_9.2.2-py_3.8
ARG VT_TV_TESTS_ENABLED=OFF
ARG VT_TV_COVERAGE_ENABLED=OFF
ARG VT_TV_TEST_PYTHON_BINDINGS=OFF

FROM ${BASE_IMAGE} AS base

# setup requirements for rendering tests (xvfb) + coverage report (lcov)
RUN apt-get update && apt-get install -y

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
RUN mkdir -p /opt/build/vt-tv/test_output
RUN bash /opt/src/vt-tv/ci/python_build.sh
RUN bash /opt/src/vt-tv/ci/python_test.sh

# Artifacts
FROM scratch AS artifacts
COPY --from=test-cpp /tmp/artifacts /tmp/artifacts
COPY --from=test-python /opt/build/vt-tv/test_output /tmp/python-artifacts
