#!/usr/bin/env bash

#
# TODO: add more options
#

set -e

SELF=${0%/*}
BIN=$(ls -t /usr/bin/clang-format* | head -1)
CONFIG=${1:-"$SELF/bc.clang-format"}
SRCDIR=${2:-"$(readlink -f "$SELF/../src")"}

find $SRCDIR -type f | \
    fgrep -vf <(\
        echo src/third_party
        echo src/benchmark
    ) | \
    xargs -I{} $SHELL -c "diff -u {} <($BIN --style=\"{ $(sed -e 's/#.*//' -e '/---/d' -e '/\.\.\./d' $CONFIG | tr $'\n' ,) }\" < {})"
