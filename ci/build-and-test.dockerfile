FROM pierrpebay/vt-tv:master AS build

ENV CC=/usr/bin/gcc-11 \
  CXX=/usr/bin/g++-11

# setup virtual X11 for tests + lcov for coverage
RUN apt-get update \
  && DEBIAN_FRONTEND="noninteractive" apt-get install -y \
    xvfb \
    lcov

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv

# build
RUN chmod +x /opt/src/vt-tv/build.sh
RUN VTK_DIR=/opt/build/vtk-build \
    VT_TV_BUILD_TYPE=Release \
    VT_TV_BUILD_DIR=/opt/build/vt-tv \
    VT_TV_COVERAGE_ENABLED=ON \
    /opt/src/vt-tv/build.sh

FROM build AS test

# test
RUN bash /opt/src/vt-tv/ci/test.sh

FROM scratch AS export-stage
COPY --from=test /tmp/artifacts /tmp/artifacts
