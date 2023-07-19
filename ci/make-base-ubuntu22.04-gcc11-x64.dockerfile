FROM ubuntu:22.04 AS base

RUN apt-get update \
  && DEBIAN_FRONTEND="noninteractive" apt-get install -y \
      git \
      python3 \
      python3-pip \
      python3-distutils \
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
     perl \
  && rm -rf /var/lib/apt/lists/*

RUN pip install nanobind

# Clone VTK source
RUN mkdir -p /opt/src/vtk
RUN git clone --recursive --branch v9.2.2 https://gitlab.kitware.com/vtk/vtk.git /opt/src/vtk

# Build VTK
RUN mkdir -p /opt/build/vtk-build
WORKDIR /opt/build/vtk-build
RUN cmake \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DBUILD_TESTING:BOOL=OFF \
  -DVTK_Group_Rendering:BOOL=OFF \
  -DBUILD_TESTING:BOOL=OFF \
  -DBUILD_SHARED_LIBS:BOOL=ON \
  -S /opt/src/vtk -B /opt/build/vtk-build
RUN cmake --build /opt/build/vtk-build -j$(nproc)

RUN echo "Base creation success"
