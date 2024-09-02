# Docker instructions to build an image with some arguments to specify compilers, python and VTK versions.
# @see .github/workflows/pushbasedockerimage.yml

ARG BASE_IMAGE=ubuntu:22.04

# Base image & requirements
FROM ${BASE_IMAGE} AS base

# Arguments
ARG VTK_VERSION=9.2.2
ARG PYTHON_VERSION=3.8
ARG CC=gcc-11
ARG CXX=g++-11
ARG GCOV=gcov-11

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update -y -q && \
  apt-get install -y -q --no-install-recommends \
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
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  mesa-common-dev \
  libosmesa6-dev \
  perl \
  curl \
  xvfb \
  lcov  \
  && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/*

# Share environment variables for use in images based on this.
ENV CC=/usr/bin/$CC
ENV CXX=/usr/bin/$CXX
ENV GCOV=/usr/bin/$GCOV
ENV VTK_DIR=/opt/build/vtk

# Setup python 3.8 with conda

# Download and install Miniconda
RUN curl -LO https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh && \
    bash Miniconda3-latest-Linux-x86_64.sh -b -p /opt/conda && \
    rm Miniconda3-latest-Linux-x86_64.sh

# Update PATH so that conda and the installed packages are usable
ENV PATH="/opt/conda/bin:${PATH}"

# Create a new environment and install necessary packages
RUN conda create -y -n deves python=${PYTHON_VERSION} && \
    echo "source activate deves" > ~/.bashrc && \
    /bin/bash -c ". /opt/conda/etc/profile.d/conda.sh && conda activate deves && pip install nanobind"

# Set the environment to deves on container run
ENV CONDA_DEFAULT_ENV=deves
ENV CONDA_PREFIX=/opt/conda/envs/$CONDA_DEFAULT_ENV
ENV PATH=$PATH:$CONDA_PREFIX/bin
ENV CONDA_AUTO_UPDATE_CONDA=false

# Clone VTK source
RUN mkdir -p /opt/src/vtk
RUN git clone --recursive --branch v${VTK_VERSION} https://gitlab.kitware.com/vtk/vtk.git /opt/src/vtk

# Build VTK
RUN mkdir -p /opt/scripts
COPY ci/build_vtk.sh /opt/scripts/build_vtk.sh
RUN VTK_DIR=${VTK_DIR} bash /opt/scripts/build_vtk.sh

RUN echo "Base creation success"
