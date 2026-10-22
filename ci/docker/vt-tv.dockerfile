ARG REPO=lifflander1/vt
ARG ARCH=amd64
ARG IMAGE=wf-amd64-ubuntu-22.04-gcc-12-vtk-cpp

ARG BASE_IMAGE=${REPO}:${IMAGE}

FROM --platform=${ARCH} ${BASE_IMAGE} AS build

# Existing base images may predate the external JSON dependency.
RUN apt-get update && \
    apt-get install -y --no-install-recommends nlohmann-json3-dev && \
    rm -rf /var/lib/apt/lists/*

ARG VT_TV_COVERAGE_ENABLED
ENV VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED

ENV CONDA_PATH=/opt/conda
ENV PATH=$CONDA_PATH/bin:$PATH
ENV CONDA_ROOT=${CONDA_PATH}
ENV CONDA_PLUGINS_AUTO_ACCEPT_TOS=true

ARG IMAGE
ARG CACHE_ID=${IMAGE}

# Keep pipeline failures visible while saving Python diagnostics in the build cache.
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

RUN --mount=type=cache,id=conda-${CACHE_ID},target=${CONDA_PATH},sharing=locked \
    --mount=type=cache,id=build-${CACHE_ID},target=/opt/build/vt-tv \
    --mount=target=/opt/src/vt-tv,rw \
        /opt/src/vt-tv/ci/setup_conda.sh && \
        /opt/src/vt-tv/ci/build_cpp.sh /opt/src/vt-tv /opt/build && \
        /opt/src/vt-tv/ci/test_cpp.sh /opt/src/vt-tv /opt/build && \
        /opt/src/vt-tv/ci/python_build.sh 2>&1 | tee /opt/build/vt-tv/python-build.log && \
        /opt/src/vt-tv/ci/python_test.sh 2>&1 | tee /opt/build/vt-tv/python-tests.log
