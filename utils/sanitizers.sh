#!/usr/bin/env bash

#
# Compile and run under different sanitizers, that compiler have (we will use
# clang).
# XXX: add valgrind somewhere too
#

set -e

self="$PWD/$0" && self="${self%/*}/"
root="$(readlink -f "$self/../")"
pathBak="$PATH"
symbolizer=$(ls -t /usr/bin/llvm-symbolizer-* | head -1)
sanitizers=(SANITIZETHREAD SANITIZEADDRESS)

function log()
{
    echo "# --------------------- " $* " -------------------- #"
}

log "Using symbolizer: $symbolizer"
for name in ${sanitizers[@]}; do
    log $name

    dir="$root/.test-$name"
    rm -fr "$dir"
    mkdir -p "$dir"
    cd "$dir"
    buildDir="$PWD/build/"

    cmake \
        -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang \
        -DCMAKE_BUILD_TYPE=$name -DBOOSTCACHE_BUILD_DIR="$buildDir" \
        $root
    make

    env MSAN_SYMBOLIZER_PATH="$symbolizer" \
        ASAN_SYMBOLIZER_PATH="$symbolizer" \
        $self/run_tests.sh "$buildDir/boostcached"
done
