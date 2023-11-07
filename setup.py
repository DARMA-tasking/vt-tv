from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import setuptools
import nanobind

class get_nanobind_include(object):
  """Helper class to determine the nanobind include path
  The purpose of this class is to postpone importing nanobind
  until it is actually installed, so that the `get_include()`
  method can be invoked."""

  def __str__(self):
    import nanobind
    return nanobind.get_include()

ext_modules = [
  Extension(
    'vttv',
    ['bindings/python/tv.cpp'],
    include_dirs=[
      # Path to nanobind headers
      get_nanobind_include(),
      './'  # Assuming the root of your C++ project is the current directory
    ],
    language='c++'
  ),
]

setup(
  name='vttv',
  version='0.0.1',
  author='Pierre Pebay',
  author_email='pierre.pebat@ng-analytics.com',
  url='https://github.com/DARMA-tasking/vt-tv',
  description='Virtual Transport Task Visualizer',
  long_description='',
  ext_modules=ext_modules,
  setup_requires=['nanobind>=1.3.2'],
  cmdclass={'build_ext': build_ext},
  zip_safe=False,
)