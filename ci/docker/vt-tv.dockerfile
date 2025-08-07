ARG REPO=lifflander1/vt
ARG ARCH=amd64
ARG IMAGE=wf-amd64-ubuntu-22.04-gcc-12-vtk-cpp

ARG BASE_IMAGE=${REPO}:${IMAGE}

FROM --platform=${ARCH} ${BASE_IMAGE} AS build

ARG VT_TV_COVERAGE_ENABLED
ENV VT_TV_COVERAGE_ENABLED=$VT_TV_COVERAGE_ENABLED

ENV CONDA_PATH=/opt/conda
ENV PATH=$CONDA_PATH/bin:$PATH
ENV CONDA_ROOT=${CONDA_PATH}
ENV CONDA_PLUGINS_AUTO_ACCEPT_TOS=true

ARG IMAGE
ARG CACHE_ID=${IMAGE}

RUN --mount=type=cache,id=conda-${CACHE_ID},target=${CONDA_PATH},sharing=locked \
    --mount=target=/opt/src/vt-tv,rw \
    /opt/src/vt-tv/ci/setup_conda.sh && \
    /opt/src/vt-tv/ci/test.sh && \
    /opt/src/vt-tv/ci/python_build.sh && \
    /opt/src/vt-tv/ci/python_test.sh


