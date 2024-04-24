FROM pierrpebay/vt-tv:master AS build

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv/test_output

# setup environment
ENV VTTV_VTK_DIR=/opt/build/vtk-build
ENV CC=gcc-11
ENV CXX=g++-11

# build bindings
RUN /bin/bash -c ". /opt/conda/etc/profile.d/conda.sh && conda activate deves && pip install /opt/src/vt-tv"

# test bindings

# create output directory
RUN mkdir -p /opt/build/vt-tv/test_output

RUN /bin/bash -c ". /opt/conda/etc/profile.d/conda.sh && conda activate deves && pip install PyYAML && python /opt/src/vt-tv/tests/test_bindings.py"
