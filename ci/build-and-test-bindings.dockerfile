FROM pierrpebay/vt-tv:master AS build

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv/test_output

# setup environment
ENV VTK_DIR=/opt/build/vtk-build
ENV CC=gcc-11
ENV CXX=g++-11

# setup virtual X11 for tests
RUN apt-get update \
  && DEBIAN_FRONTEND="noninteractive" apt-get install -y \
      xvfb \
  && rm -rf /var/lib/apt/lists/*

# create output directory
RUN mkdir -p /opt/build/vt-tv/test_output

# test
RUN ["chmod", "+x", "/opt/src/vt-tv/ci/test-bindings.sh"]
RUN ["/bin/sh", "/opt/src/vt-tv/ci/test-bindings.sh"]
