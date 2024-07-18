ARG BASE=ubuntu:22.04

# Base image & requirements
FROM ${BASE} AS base

ARG CC=gcc-11
ARG CXX==g++-11
ARG VTK_TAG=v9.2.2
ARG VTK_DIR=/opt/build/vtk-build
ARG PYTHON=3.8

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
RUN export CC="\$(which ${CC})"
RUN export CXX="\$(which ${CXX})"

# Setup python with conda
RUN \
  # Download and install Miniconda
  curl -LO https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh && \
  bash Miniconda3-latest-Linux-x86_64.sh -b -p /opt/conda && \
  rm Miniconda3-latest-Linux-x86_64.sh \
  \
  # Update PATH so that conda and the installed packages are usable
  export PATH=/opt/conda/bin:\$PATH \
  \
  # Create a new environment and install necessary packages
  RUN conda create -y -n deves python=${PYTHON} && \
  echo "source activate deves" > ~/.bashrc && \
  /bin/bash -c ". /opt/conda/etc/profile.d/conda.sh && conda activate deves && pip install nanobind" \
  \
  # Set the environment to deves on container run
  export CONDA_DEFAULT_ENV=deves \
  export CONDA_PREFIX=/opt/conda/envs/$CONDA_DEFAULT_ENV \
  export PATH=$PATH:$CONDA_PREFIX/bin \
  export CONDA_AUTO_UPDATE_CONDA=false

# Clone VTK source
RUN mkdir -p /opt/src/vtk
RUN git clone --recursive --branch ${VTK_TAG} https://gitlab.kitware.com/vtk/vtk.git /opt/src/vtk

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

# Build
FROM base AS build

ARG VTK_DIR

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv

RUN chmod +x /opt/src/vt-tv/build.sh
RUN CMAKE_BINARY_DIR=/opt/build/vt-tv \
    VTK_DIR=${VTK_DIR} \
    VT_TV_TESTS_ENABLED=ON \
    VT_TV_COVERAGE_ENABLED=ON \
    /opt/src/vt-tv/build.sh

RUN echo "VT-TV build success"

# Unit tests
FROM build AS test
RUN ["chmod", "+x", "/opt/src/vt-tv/ci/test.sh"]
RUN ["/bin/sh", "/opt/src/vt-tv/ci/test.sh"]
RUN bash /opt/src/vt-tv/ci/docker/test.sh

# Bindings tests
FROM build AS test-bindings
RUN ["chmod", "+x", "/opt/src/vt-tv/ci/test-bindings.sh"]
RUN ["/bin/sh", "/opt/src/vt-tv/ci/test-bindings.sh"]

# Artifacts
FROM scratch AS artifacts
COPY --from=test /tmp/artifacts /tmp/artifacts
COPY --from=test-bindings /tmp/artifacts /tmp/artifacts
