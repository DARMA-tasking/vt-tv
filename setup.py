"""
Setup vt-tv as a python package using pip.

To run the script you need to set VTK_DIR environemnt variable
Example: `VTK_DIR=../vtk/build pip install .`
"""
import subprocess
import os
import sys
import shutil

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext


# Ensure python-build directory exists
BUILD_DIR = 'python-build'
if os.path.exists(BUILD_DIR):
    shutil.rmtree(BUILD_DIR)
os.makedirs(BUILD_DIR)


class CMakeExtension(Extension):
    """A setup extension which defines a name and a CMake source directory"""

    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):

    """A setup command to build vt-tv using CMake"""

    def run(self):
        """Run build"""

        try:
            __out = subprocess.check_output(['cmake', '--version'])
        except OSError as err:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions)) from err

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        """Builds the extension"""

        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        # The following fix output of /home/thomas/repositories/vt-tv/build/lib.linux-x86_64-cpython-38
        # but then are not copied to site-packages/ There should be additional thing to do
        # extdir = os.path.join(BUILD_DIR, 'build', 'ext')
        build_temp = os.path.join(BUILD_DIR, 'build', 'temp')
        os.makedirs(build_temp, exist_ok=True)

        vtk_dir = os.environ.get('VTK_DIR')
        if not vtk_dir:
            raise RuntimeError("Environment variable VTK_DIR is required")

        jobs = os.environ.get('VTTV_CMAKE_JOBS', os.cpu_count())

        n_threads = os.environ.get('VTTV_N_THREADS', 1)
        # check if n_threads is a valid integer
        try:
            n_threads = int(n_threads)
        except ValueError as err:
            raise RuntimeError(
                "Environment variable VTTV_N_THREADS must be an integer") from err

        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable,
                      '-DVTK_DIR=' + vtk_dir,
                      '-DVT_TV_N_THREADS=' + str(n_threads)]

        if sys.platform == "darwin":
            import platform # pylint:disable=C0415:import-outside-toplevel
            macos_version = platform.mac_ver()[0]
            cmake_args.append(
                '-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=' + macos_version)

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        os.chdir(build_temp)
        self.spawn(['cmake', ext.sourcedir] + cmake_args)
        if not self.dry_run:
            self.spawn(['cmake', '--build', '.', '--parallel',
                       '-j' + str(jobs)] + build_args)
        os.chdir(ext.sourcedir)

setup(
    name='vttv',
    version='0.0.1',
    author='Pierre Pebay',
    author_email='pierre.pebay@ng-analytics.com',
    url='https://github.com/DARMA-tasking/vt-tv',
    description='Virtual Transport Task Visualizer',
    long_description='',
    ext_modules=[CMakeExtension('vttv', sourcedir='.')],
    cmdclass=dict(build_ext=CMakeBuild),
    package_dir={'': BUILD_DIR},
    packages=find_packages(BUILD_DIR),
    zip_safe=False
)
