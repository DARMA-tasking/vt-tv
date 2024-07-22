# Docker instructions to build an image with some arguments to specify compilers, python and VTK versions.
# @see .github/workflows/pushbasedockerimage.yml

# Arguments
ARG BASE_IMAGE=ubuntu:22.04
ARG CC=gcc-11
ARG CXX=g++-11
ARG VTK_VERSION=v9.2.2
ARG PYTHON_VERSION=3.8

# Base image & requirements
FROM ${BASE_IMAGE} AS base

ARG CC CXX VTK_VERSION VTK_DIR PYTHON_VERSION

ENV DEBIAN_FRONTEND=noninteractive
ENV VTK_DIR=/opt/build/vtk-build

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

# Put CC and CXX in env for CMake
# Note: `export` is needed because command is run from another container
# ENV export CC="$(which ${CC})"
# ENV export CXX="$(which ${CXX})"

# If ENV export ... not correctly passed to the ENV of the dependent image, try
RUN mkdir /vol1
RUN echo "CC=\$(which ${CC})" >> /vol1/.env
RUN echo "CXX=\$(which ${CXX})" >> /vol1/.env
RUN echo "VTK_DIR=$VTK_DIR" >> /vol1/.env

# Load variables to bash (CC, CXX, VTK_DIR)
RUN /bin/bash -c "set -a && source /vol1/.env && set +a"

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
RUN mkdir -p ${VTK_DIR}
WORKDIR ${VTK_DIR}
RUN cmake \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DBUILD_TESTING:BOOL=OFF \
  -DVTK_OPENGL_HAS_OSMESA:BOOL=ON \
  -DVTK_DEFAULT_RENDER_WINDOW_OFFSCREEN:BOOL=ON \
  -DVTK_USE_X:BOOL=OFF \
	-DVTK_USE_WIN32_OPENGL:BOOL=OFF \
	-DVTK_USE_COCOA:BOOL=OFF \
	-DVTK_USE_SDL2:BOOL=OFF \
  -DVTK_Group_Rendering:BOOL=OFF \
  -DBUILD_SHARED_LIBS:BOOL=ON \
  -S /opt/src/vtk -B ${VTK_DIR}
RUN cmake --build ${VTK_DIR} -j$(nproc)

RUN echo "Base creation success"
