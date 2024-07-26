ARG BASE_IMAGE=pierrpebay/vt-tv:ubuntu_22.04-gcc_11-vtk_9.2.2-py_3.8
ARG VT_TV_TESTS_ENABLED=OFF
ARG VT_TV_COVERAGE_ENABLED=OFF
ARG VT_TV_PYTHON_BINDINGS_ENABLED=OFF
ARG GCOV=gcov

FROM ${BASE_IMAGE} AS base

ENV GCOV=$GCOV

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
ARG VT_TV_COVERAGE_ENABLED=OFF
ARG VT_TV_TESTS_ENABLED=OFF
RUN VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED bash /opt/src/vt-tv/ci/test_cpp.sh

# Python tests (Builds VT-TV with Python bindings & test python package)
FROM base AS test-python
ARG VT_TV_PYTHON_BINDINGS_ENABLED=OFF
RUN if [[ VT_TV_PYTHON_BINDINGS_ENABLED == "ON" ]]; then \n \
    bash /opt/src/vt-tv/ci/test_python.sh \n \
    fi

# Artifacts
FROM scratch AS artifacts
COPY --from=test-cpp /tmp/artifacts /tmp/artifacts
