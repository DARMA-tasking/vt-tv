ARG BASE_IMAGE=pierrpebay/vt-tv:ubuntu_22.04-gcc_11-vtk_9.2.2-py_3.8

FROM ${BASE_IMAGE} AS build

ENV CC=/usr/bin/gcc-11 \
  CXX=/usr/bin/g++-11

# setup virtual X11 for tests + lcov for coverage
RUN apt-get update \
  && DEBIAN_FRONTEND="noninteractive" apt-get install -y \
    xvfb \
    lcov

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv

# Unit tests
FROM build AS test-cpp
RUN ["chmod", "+x", "/opt/src/vt-tv/ci/test_cpp.sh"]
RUN ["/bin/sh", "/opt/src/vt-tv/ci/test_cpp.sh"]
RUN bash /opt/src/vt-tv/ci/docker/test.sh

# Bindings tests
FROM build AS test-python
RUN ["chmod", "+x", "/opt/src/vt-tv/ci/test_python.sh"]
RUN ["/bin/sh", "/opt/src/vt-tv/ci/test_python.sh"]

# Artifacts
FROM scratch AS artifacts
COPY --from=test /tmp/artifacts /tmp/artifacts
COPY --from=test-bindings /tmp/artifacts /tmp/artifacts
