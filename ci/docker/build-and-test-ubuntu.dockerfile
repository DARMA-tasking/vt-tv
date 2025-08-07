ARG REPO=lifflander1/vt
ARG ARCH=amd64
ARG IMAGE=wf-amd64-ubuntu-22.04-gcc-12-vtk-cpp

ARG BASE_IMAGE=${REPO}:${IMAGE}

FROM --platform=${ARCH} ${BASE_IMAGE} AS base

ARG VT_TV_COVERAGE_ENABLED=OFF
ENV VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED

COPY . /opt/src/vt-tv

ENV CONDA_PATH=/opt/conda
ENV PATH=$CONDA_PATH/bin:$PATH

RUN /opt/src/vt-tv/ci/setup_conda.sh

RUN mkdir -p /opt/build/vt-tv

# Build
FROM base AS build

ARG IMAGE
ARG CACHE_ID=${IMAGE}
ARG VT_TV_TESTS_ENABLED=OFF

RUN --mount=type=cache,id=BUILD-${CACHE_ID},target=/opt/src/vt-tv/output \
    /opt/src/vt-tv/ci/build.sh

# Unit tests
FROM build AS test-cpp

ARG IMAGE
ARG CACHE_ID=${IMAGE}
ARG VT_TV_TESTS_ENABLED=OFF

RUN --mount=type=cache,id=BUILD-${CACHE_ID},target=/opt/src/vt-tv/output \
    /opt/src/vt-tv/ci/test.sh

# Python tests (Builds VT-TV with Python bindings & test python package)
FROM test-cpp AS test-python

ARG IMAGE
ARG CACHE_ID=${IMAGE}

# Create vizualization output directory (required)
RUN --mount=type=cache,id=BUILD-${CACHE_ID},target=/opt/src/vt-tv/output \
    mkdir -p /opt/src/vt-tv/output/python_tests && \
    /opt/src/vt-tv/ci/python_build.sh && \
    /opt/src/vt-tv/ci/python_test.sh

# Artifacts
FROM scratch AS artifacts
COPY --from=test-cpp /tmp/artifacts /tmp/artifacts
COPY --from=test-python /opt/src/vt-tv/output/python_tests /tmp/python-artifacts
COPY --from=build /opt/build/vt-tv/install /tmp/pkg-artifacts
