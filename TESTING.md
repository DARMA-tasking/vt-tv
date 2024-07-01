## Unit Testing

Unit tests are implemented with the help of the [Google's C++ test framework](https://github.com/google/googletest).
The tests source files are located under the `tests` directory as test_*.cpp.

You might also add an example in the `example` directory which will be also run as a test as defined by the current cmake configuration.

To run the test from the CLI first build vttv then call ctest using the command line:
`ctest --test-dir build`

To run specific tests you can use the ctest search option:
`ctest --test-dir build --verbose -R "reader"`
