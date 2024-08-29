ARG BASE_IMAGE=lifflander1/vt:ubuntu_22.04-gcc_11-vtk_9.2.2-py_3.8
ARG VT_TV_TESTS_ENABLED=OFF
ARG VT_TV_COVERAGE_ENABLED=OFF
ARG VT_TV_TEST_PYTHON_BINDINGS=OFF

FROM ${BASE_IMAGE} AS base

# setup requirements for rendering tests (xvfb) + coverage report (lcov)
RUN apt-get update && apt-get install -y \
    xvfb \
    lcov

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv

# Build
FROM base AS build
ARG VT_TV_COVERAGE_ENABLED=OFF
ARG VT_TV_TESTS_ENABLED=OFF
RUN VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED bash /opt/src/vt-tv/ci/build.sh

# Unit tests
FROM build AS test-cpp
ENV TEST_PHASE=cpp
ARG VT_TV_COVERAGE_ENABLED=OFF
ARG VT_TV_TESTS_ENABLED=OFF
RUN VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED bash /opt/src/vt-tv/ci/test_cpp.sh

# Python tests (Builds VT-TV with Python bindings & test python package)
FROM base AS test-python
ENV TEST_PHASE=python
# Create vizualization output directory (required)
RUN mkdir -p /opt/build/vt-tv/test_output
RUN bash /opt/src/vt-tv/ci/test_python.sh

FROM base AS all-tests-output
COPY --from=test-cpp /tmp/artifacts /tmp/artifacts
COPY --from=test-python /opt/build/vt-tv/test_output /tmp/python-artifacts

# Artifacts
FROM scratch AS artifacts
COPY --from=all-tests-output /tmp/artifacts /tmp/artifacts
COPY --from=all-tests-output /tmp/python-artifacts /tmp/python-artifacts
