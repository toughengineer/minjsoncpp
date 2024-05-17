# Tests

[Catch2](https://github.com/catchorg/Catch2) is used as the testing framework.

Make sure to init Catch2 submodule:
```
git submodule update --init
```

To build tests specify `MINJSONCPP_BUILD_TESTS` CMake cache variable.

To run tests execute
```
test
```

To run benchmarks execute
```
test [!benchmark]
```

To get help on command line options execute
```
test -?
```