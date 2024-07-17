FROM pierrpebay/vt-tv:master AS build

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv

# build
RUN chmod +x /opt/src/vt-tv/build.sh
RUN /opt/src/vt-tv/build.sh \
    --build-dir=/opt/build/vt-tv \
    --vtk-dir=/opt/build/vtk-build \
    --cc=gcc-11 \
    --cxxc=g++-11 \
    --coverage \
    --tests

FROM build AS test

# test
RUN bash /opt/src/vt-tv/ci/test.sh

FROM scratch AS export-stage
COPY --from=test /tmp/artifacts /tmp/artifacts
