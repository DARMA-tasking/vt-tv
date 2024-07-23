ARG BASE_IMAGE=pierrpebay/vt-tv:ubuntu_22.04-gcc_11-vtk_9.2.2-py_3.8

FROM ${BASE_IMAGE} AS build

# setup requirements for rendering tests (xvfb) + coverage report (lcov)
RUN apt-get update && apt-get install -y \
    xvfb \
    lcov

COPY . /opt/src/vt-tv
RUN mkdir -p /opt/build/vt-tv

# Build
RUN if [[ $CC == "gcc-11" ]]; then \
        bash VT_TV_COVERAGE_ENABLED=ON /opt/src/vt-tv/ci/build.sh \
    else \
        bash VT_TV_COVERAGE_ENABLED=OFF /opt/src/vt-tv/ci/build.sh \
    fi

# Unit tests
FROM build AS test-cpp
RUN bash /opt/src/vt-tv/ci/test_cpp.sh

# Python tests (Builds VT-TV with Python bindings & test python package)
FROM build AS test-python
RUN bash "/opt/src/vt-tv/ci/test_python.sh"

# Artifacts
FROM scratch AS artifacts
COPY --from=test-cpp /tmp/artifacts /tmp/artifacts
