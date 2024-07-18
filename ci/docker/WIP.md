WIP: Configurable docker image to test multiple configurations (os, compiler, vtk version, python version for bindings)
See also workflow pushbasedockerimage-config.yml

The goal is to make the base image more scalable with some matrix in pipeline to build and test with different configurations and to push to dockerhub.

VTK is very long to build so it is important to generate different configurations on DOCKERHUB (excluding tests and coverage) with VTK already built.
