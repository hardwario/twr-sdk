#!/bin/bash
set -eu
_step_counter=0

: ${TEST_DIR:="$(pwd)/build_examples"}
: ${EXAMPLES_DIR:="$(pwd)/_examples"}

step() {
	_step_counter=$(( _step_counter + 1 ))
	printf '\n\033[1;36m%d) %s\033[0m\n' $_step_counter "$@" >&2  # bold cyan
}

step 'Prepare'
rm -rf "$TEST_DIR"
echo "$TEST_DIR"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

mkdir sdk
cd sdk
cp -r ../../bcl bcl
cp -r ../../twr twr
cp -r ../../lib lib
cp -r ../../fonts fonts
cp -r ../../stm stm
cp -r ../../toolchain toolchain
cp -r ../../tools tools
cp -r ../../sys sys
cp -r ../../CMakeLists.txt CMakeLists.txt
cd ..

echo """
cmake_minimum_required(VERSION 3.20.0)

# Setup project name and languages
project(firmware LANGUAGES C ASM)

add_subdirectory(sdk)

# If you need to add some source files to the project add them to the 'src' folder and update CMakeLists there
add_subdirectory(src)
""" > CMakeLists.txt

for dir in $EXAMPLES_DIR/*/
do
    rm -rf src
    rm -rf obj/debug/
    rm -rf out/debug/
    dir=${dir%*/}
    step "Test ${dir##*/}"
    cp -r $dir src
    echo """
# List any additional sources here
target_sources(
    \${CMAKE_PROJECT_NAME}
    PUBLIC
    application.c
    )

# If you added some folder with header files you need to list them here
target_include_directories(
    \${CMAKE_PROJECT_NAME}
    PUBLIC
    \${CMAKE_CURRENT_SOURCE_DIR}
)
""" > src/CMakeLists.txt
    cmake -B obj/debug . -G Ninja -DCMAKE_TOOLCHAIN_FILE=sdk/toolchain/toolchain.cmake -DTYPE=debug
    ninja -C obj/debug
done

step 'Clean'
rm -rf "$TEST_DIR"
