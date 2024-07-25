## Unit Testing

### Running unit tests

Unit tests are implemented using thhe [Google's C++ test framework](https://github.com/google/googletest).
The tests source files are located under the `tests` directory as test_*.cpp.

To build and test vt-tv use the `build.sh` script
Type `build.sh --help` to get started.

You might want also directly run the tests from the build:
`{VT_TV_BUID_DIR}/tests/unit/AllTests`
Then you can run test with some google test options as described at [Google Test - Running Test Programs: Advanced Options](https://google.github.io/googletest/advanced.html#running-test-programs-advanced-options)
