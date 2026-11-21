default:
    @just --list

# Development workflow (build/)

# configures the build system for development mode
configure:
    cmake --preset debug

# builds the program in development mode
build:
    cmake --build --preset debug

# builds the debug test executable
build-tests:
    cmake --build --preset debug --target unit-tests

# runs all debug tests
test: build-tests
    ctest --preset debug --no-tests=error

# runs tests matching a Catch2 tag, for example: just test-tag '[core]'
test-tag TAG: build-tests
    ./build/tests/unit-tests {{quote(TAG)}}

# runs the program with gbd attached for debugging
run-debugger: build
    gdb -q -ex "set debuginfod enabled on" -ex "set confirm off" -ex run --args ./build/hexterminator

# runs the program
run: build
    ./build/hexterminator

# Release workflow (build-release/)

# configures the build system for release mode
configure-release:
    cmake --preset release

# builds the program in release mode
build-release:
    cmake --build --preset release

# builds and runs all release tests
test-release:
    cmake --build --preset release --target unit-tests
    ctest --preset release --no-tests=error

# runs the program in release mode
run-release: build-release
    ./build-release/hexterminator

# Utils

# formats all C++ sources and headers with clang-format
format:
    clang-format -i src/*.cpp include/*.hpp

# checks if all C++ sources and headers adhere to clang-format
check-format:
    clang-format --dry-run --Werror src/*.cpp include/*.hpp

# cleans all temporary directories
clean:
    rm -rf build build-release
