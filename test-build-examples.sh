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
ln -s ../../bcl
ln -s ../../twr
ln -s ../../lib
ln -s ../../fonts
ln -s ../../stm
ln -s ../../sys
ln -s ../../Makefile.mk
cd ..

echo """
SDK_DIR ?= sdk

CFLAGS += -D'HIO_SCHEDULER_MAX_TASKS=64'

-include sdk/Makefile.mk
""" > Makefile

step 'Compile sdk'
mkdir app
make -j4

for dir in $EXAMPLES_DIR/*/
do
    rm -rf app
    rm -rf obj/debug/app
    dir=${dir%*/}
    step "Test ${dir##*/}"
    cp -r $dir app
    make -j4
done

step 'Clean'
rm -rf "$TEST_DIR"
