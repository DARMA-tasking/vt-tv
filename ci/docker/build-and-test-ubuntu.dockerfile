ARG BASE_IMAGE=lifflander1/vt:ubuntu_22.04-gcc_11-vtk_9.2.2-py_3.8
ARG VT_TV_TESTS_ENABLED=OFF
ARG VT_TV_COVERAGE_ENABLED=OFF
ARG VT_TV_TEST_PYTHON_BINDINGS=OFF

FROM ${BASE_IMAGE} AS base

ENV CONDA_PATH=/opt/conda
ENV PATH=$PATH:$CONDA_PATH/bin

# Setup python requirements for JSON datafile validation
RUN pip install PyYAML
RUN pip install Brotli
RUN pip install schema
RUN pip install nanobind

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
