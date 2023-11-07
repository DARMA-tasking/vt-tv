import os
import re
import subprocess
import sys
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext


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
    cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
            '-DPYTHON_EXECUTABLE=' + sys.executable]

    cfg = 'Debug' if self.debug else 'Release'
    build_args = ['--config', cfg]

    if not os.path.exists(self.build_temp):
      os.makedirs(self.build_temp)

    subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp)
    subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)


setup(
  name='vttv',
  version='0.0.1',
  author='Pierre Pebay',
  author_email='pierre.pebat@ng-analytics.com',
  url='https://github.com/DARMA-tasking/vt-tv',
  description='Virtual Transport Task Visualizer',
  long_description='',
  ext_modules=ext_modules,
  setup_requires=['nanobind>=1.3.2', 'cmake>=3.17.0'],
  packages=find_packages(),
  ext_modules=[CMakeExtension('vttv', 'bindings/python')],
  cmdclass=dict(build_ext=CMakeBuild),
  zip_safe=False,
)