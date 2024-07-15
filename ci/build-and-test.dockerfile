FROM pierrpebay/vt-tv:master AS build

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv

# add clang and clang++ as alternative compiler
RUN apt-get update \
  && apt-get install -y clang-15

# build
RUN bash /opt/src/vt-tv/ci/build.sh

FROM build AS test

# test
RUN bash /opt/src/vt-tv/ci/test.sh

FROM scratch AS export-stage
COPY --from=test /tmp/artifacts /tmp/artifacts
