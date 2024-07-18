FROM pierrpebay/vt-tv:master AS build

# setup virtual X11 for tests + lcov for coverage
RUN apt-get update \
  && DEBIAN_FRONTEND="noninteractive" apt-get install -y \
    xvfb \
    lcov

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv

# build
RUN chmod +x /opt/src/vt-tv/build.sh
RUN VT_TV_BUILD_TYPE=Release\
    VT_TV_COVERAGE_ENABLED=1 \
    ./build.sh

FROM build AS test

# test
RUN bash /opt/src/vt-tv/ci/test.sh

FROM scratch AS export-stage
COPY --from=test /tmp/artifacts /tmp/artifacts
