# Docker instructions to build an image with some arguments to specify compilers, python and VTK versions.
# @see .github/workflows/pushbasedockerimage.yml

ARG BASE_IMAGE=ubuntu:22.04

# Base image & requirements
FROM ${BASE_IMAGE} AS base

# Arguments
ARG VTK_VERSION=9.2.2
ARG PYTHON_VERSIONS=3.8,3.9,3.10,3.11,3.12
ARG CC=gcc-11
ARG CXX=g++-11
ARG GCOV=gcov-11

# Copy setup scripts
RUN mkdir -p /opt/scripts
COPY ci/setup_mesa.sh /opt/scripts/setup_mesa.sh
COPY ci/setup_mesa.sh /opt/scripts/setup_conda.sh
COPY ci/setup_vtk.sh /opt/scripts/setup_vtk.sh

ENV DEBIAN_FRONTEND=noninteractive

# Setup common tools and compiler
RUN apt update -y -q && \
  apt install -y -q --no-install-recommends \
  ${CC} \
  ${CXX} \
  git \
  xz-utils \
  bzip2 \
  zip \
  gpg \
  wget \
  gpgconf \
  software-properties-common \
  libsigsegv2 \
  libsigsegv-dev \
  pkg-config \
  zlib1g \
  zlib1g-dev \
  m4 \
  gfortran-11 \
  make \
  cmake-data \
  cmake \
  pkg-config \
  libncurses5-dev \
  m4 \
  perl \
  curl \
  xvfb \
  lcov

# Setup MESA (opengl)
RUN bash /opt/scripts/setup_mesa.sh
RUN xvfb-run bash -c "glxinfo | grep 'OpenGL version'"

# Setup conda and python environments
RUN bash /opt/scripts/setup_conda.sh ${PYTHON_VERSIONS}

# Setup compiler using environment variables
ENV CC=/usr/bin/$CC
ENV CXX=/usr/bin/$CXX
ENV GCOV=/usr/bin/$GCOV

# Setup VTK
RUN VTK_VERSION=${VTK_VERSION} \
  VTK_DIR=${VTK_DIR} \
  VTK_SRC_DIR=/opt/src/vtk \
  bash /opt/scripts/setup_vtk.sh

# Clean apt
RUN apt clean && rm -rf /var/lib/apt/lists/*

RUN echo "Base creation success"
