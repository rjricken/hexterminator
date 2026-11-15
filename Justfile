default:
    @just --list

# Development workflow (build/)

# configures the build system for development mode
configure:
    cmake --preset debug

# builds the program in development mode
build:
    cmake --build --preset debug

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

# runs the program in release mode
run-release: build-release
    ./build-release/hexterminator

# Utils

# cleans all temporary directories
clean:
    rm -rf build build-release