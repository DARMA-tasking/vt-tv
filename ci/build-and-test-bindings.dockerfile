FROM pierrpebay/vt-tv:master AS build

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv/test_output

# setup environment
ENV VTK_DIR=/opt/build/vtk-build
ENV CC=gcc-11
ENV CXX=g++-11

# setup virtual X11
RUN apt-get update \
  && apt-get install -y \
  && DEBIAN_FRONTEND="noninteractive" apt-get install -y \
      xvfb \
  && rm -rf /var/lib/apt/lists/*

ENV DISPLAY=:99.0
RUN Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &

# build bindings
RUN /bin/bash -c ". /opt/conda/etc/profile.d/conda.sh && conda activate deves && pip install /opt/src/vt-tv"

# test bindings

# create output directory
RUN mkdir -p /opt/build/vt-tv/test_output

RUN /bin/bash -c ". /opt/conda/etc/profile.d/conda.sh && conda activate deves && pip install PyYAML && python /opt/src/vt-tv/tests/test_bindings.py"
