Images are built in the CI

To build image locally here is an example call using available build arguments:

For the base image
```shell
docker build -t vttv:latest --build-arg BASE_IMAGE="ubuntu:24.04" --build-arg GCOV=gcov-14 --build-arg BASE_IMAGE=ubuntu:24.04 --build-arg CC=gcc-14 --build-arg CXX=g++-14 --build-arg VTK=9.3.1 -f make-base.dockerfile .
```
Then for the build & test image
```shell
docker build -t vttv-build:latest --build-arg BASE_IMAGE="vttv:latest" --build-arg VT_TV_TESTS_ENABLED=ON --build-arg VT_TV_COVERAGE_ENABLED=OFF --build-arg VT_TV_PYTHON_BINDINGS_ENABLED=ON -f build-and-test.dockerfile .
```
