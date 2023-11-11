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
    cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
            '-DPYTHON_EXECUTABLE=' + sys.executable]

    # Add the path to the VTK installation if necessary
    cmake_args += ['-DVTK_DIR:PATH=~/Develop/vtk-build']
    cmake_args += ['-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=13.5']

    cfg = 'Debug' if self.debug else 'Release'
    build_args = ['--config', cfg]

    # Assuming the CMakeLists.txt is in the root of the project directory
    # get to running this command: cmake --build /path/to/vt-tv/source/ --parallel -j8 --target install
    os.chdir(ext.sourcedir)
    self.spawn(['cmake', ext.sourcedir] + cmake_args)
    if not self.dry_run:
      self.spawn(['cmake', '--build', '.', '--parallel', '-j8'] + build_args)
    # # Returning to the previous directory
    # print(self.distribution.get_fullname())
    # os.chdir(self.distribution.get_fullname())


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
  packages=find_packages(),
  zip_safe=False,
)