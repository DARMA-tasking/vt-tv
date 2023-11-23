from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
import subprocess
import os
import sys

class CMakeExtension(Extension):
  def __init__(self, name, sourcedir=''):
    Extension.__init__(self, name, sources=[])
    self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
  def run(self):
    try:
      out = subprocess.check_output(['cmake', '--version'])
    except OSError:
      raise RuntimeError('CMake must be installed to build the following extensions: ' +
                 ', '.join(e.name for e in self.extensions))

    for ext in self.extensions:
      self.build_extension(ext)

  def build_extension(self, ext):
    extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

    # Create a temporary build directory
    build_temp = os.path.join('python-build', 'build', 'temp')
    os.makedirs(build_temp, exist_ok=True)

    cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                  '-DPYTHON_EXECUTABLE=' + sys.executable,
                  '-DVTK_DIR:PATH=~/Develop/vtk-build',
                  '-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=13.5']

    cfg = 'Debug' if self.debug else 'Release'
    build_args = ['--config', cfg]

    # Change to the build directory
    os.chdir(build_temp)

    # Run CMake and build commands
    self.spawn(['cmake', ext.sourcedir] + cmake_args)
    if not self.dry_run:
      self.spawn(['cmake', '--build', '.', '--parallel', '-j8'] + build_args)

    # Change back to the original directory
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
  package_dir={'': 'python-build'},
  packages=find_packages('python-build'),
  zip_safe=False,
)