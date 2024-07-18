This folder is an investigation to work with test build-and-test-vt-tv-matrix.yml pipeline

The goal is to make the base image more scalable with some matrix in pipeline to build and test with different configuraions.

VTK is very long to build so it is important to generate different configurations on DOCKERHUB (excluding tests and coverage). The current tests contains all stuff (setup image + tests)

Once it work we should update the base image and make configurable build with args

to test different VTK versions and COMPILER versions
