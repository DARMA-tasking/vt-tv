from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
import subprocess
import os
import sys

# Ensure python-build directory exists
build_dir = 'python-build'
os.makedirs(build_dir, exist_ok=True)

class CMakeExtension(Extension):
  def __init__(self, name, sourcedir=''):
    Extension.__init__(self, name, sources=[])
    self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
  def run(self):
    try:
      out = subprocess.check_output(['cmake', '--version'])
    except OSError:
      raise RuntimeError("CMake must be installed to build the following extensions: " +
                         ", ".join(e.name for e in self.extensions))

    for ext in self.extensions:
      self.build_extension(ext)

  def build_extension(self, ext):
    extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
    build_temp = os.path.join('python-build', 'build', 'temp')
    os.makedirs(build_temp, exist_ok=True)

    vtk_dir = os.environ.get('VTTV_VTK_DIR')
    if not vtk_dir:
      raise RuntimeError("Environment variable VTTV_VTK_DIR is required")

    jobs = os.environ.get('VTTV_CMAKE_JOBS', os.cpu_count())

    n_threads = os.environ.get('VTTV_N_THREADS', 1)
    # check if n_threads is a valid integer
    try:
      n_threads = int(n_threads)
    except ValueError:
      raise RuntimeError("Environment variable VTTV_N_THREADS must be an integer")

    cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                  '-DPYTHON_EXECUTABLE=' + sys.executable,
                  '-DVTK_DIR=' + vtk_dir,
                  '-DVT_TV_NUM_THREADS=' + str(n_threads)]

    if sys.platform == "darwin":
      import platform
      macos_version = platform.mac_ver()[0]
      cmake_args.append('-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=' + macos_version)

    cfg = 'Debug' if self.debug else 'Release'
    build_args = ['--config', cfg]

    os.chdir(build_temp)
    self.spawn(['cmake', ext.sourcedir] + cmake_args)
    if not self.dry_run:
      self.spawn(['cmake', '--build', '.', '--parallel', '-j' + str(jobs)] + build_args)
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
  package_dir={'': build_dir},
  packages=find_packages(build_dir),
  zip_safe=False,
)
