ARG REPO=lifflander1/vt
ARG ARCH=amd64
ARG IMAGE=wf-amd64-ubuntu-22.04-gcc-12-vtk-cpp

ARG BASE_IMAGE=${REPO}:${IMAGE}

FROM --platform=${ARCH} ${BASE_IMAGE} AS base

ENV CONDA_PATH=/opt/conda
ENV PATH=$PATH:$CONDA_PATH/bin

# Setup python requirements for JSON datafile validation
RUN apt-get update && apt-get install -y python3-full python3-pip
RUN pip install --no-cache-dir PyYAML Brotli schema nanobind

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
RUN VTK_DIR=/opt/build/vtk bash /opt/src/vt-tv/ci/python_build.sh
RUN VTK_DIR=/opt/build/vtk bash /opt/src/vt-tv/ci/python_test.sh

# Artifacts
FROM scratch AS artifacts
COPY --from=test-cpp /tmp/artifacts /tmp/artifacts
COPY --from=test-python /opt/src/vt-tv/output/python_tests /tmp/python-artifacts
COPY --from=build /opt/build/vt-tv/install /tmp/pkg-artifacts
