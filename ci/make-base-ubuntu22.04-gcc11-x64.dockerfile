FROM ubuntu:22.04 AS base

RUN apt-get update \
  && DEBIAN_FRONTEND="noninteractive" apt-get install -y \
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
  && rm -rf /var/lib/apt/lists/*

RUN apt-get update \
  && apt-get install -y \
     gcc-11 \
     g++-11 \
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
  && rm -rf /var/lib/apt/lists/*

# Setup python 3.8 with conda

# Download and install Miniconda
RUN curl -LO https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh && \
    bash Miniconda3-latest-Linux-x86_64.sh -b -p /opt/conda && \
    rm Miniconda3-latest-Linux-x86_64.sh

# Update PATH so that conda and the installed packages are usable
ENV PATH="/opt/conda/bin:${PATH}"

# Create a new environment and install necessary packages
RUN conda create -y -n deves python=3.8 && \
    echo "source activate deves" > ~/.bashrc && \
    /bin/bash -c ". /opt/conda/etc/profile.d/conda.sh && conda activate deves && pip install nanobind"

# Set the environment to deves on container run
ENV CONDA_DEFAULT_ENV=deves
ENV CONDA_PREFIX=/opt/conda/envs/$CONDA_DEFAULT_ENV
ENV PATH=$PATH:$CONDA_PREFIX/bin
ENV CONDA_AUTO_UPDATE_CONDA=false

# Clone VTK source
RUN mkdir -p /opt/src/vtk
RUN git clone --recursive --branch v9.2.2 https://gitlab.kitware.com/vtk/vtk.git /opt/src/vtk

# Build VTK
RUN mkdir -p /opt/build/vtk-build
WORKDIR /opt/build/vtk-build
RUN cmake \
  -DCMAKE_C_COMPILER=gcc-11 \
  -DCMAKE_CXX_COMPILER=g++-11 \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DBUILD_TESTING:BOOL=OFF \
  -DVTK_OPENGL_HAS_OSMESA:BOOL=ON \
  -DVTK_DEFAULT_RENDER_WINDOW_OFFSCREEN:BOOL=ON \
  -DVTK_USE_X:BOOL=OFF \
	-DVTK_USE_WIN32_OPENGL:BOOL=OFF \
	-DVTK_USE_COCOA:BOOL=OFF \
	-DVTK_USE_SDL2:BOOL=OFF \
  -DVTK_Group_Rendering:BOOL=OFF \
  -DBUILD_TESTING:BOOL=OFF \
  -DBUILD_SHARED_LIBS:BOOL=ON \
  -S /opt/src/vtk -B /opt/build/vtk-build
RUN cmake --build /opt/build/vtk-build -j$(nproc)

RUN echo "Base creation success"
