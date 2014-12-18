#!/usr/bin/env bash

#
# TODO: add more options
#

set -e

SELF=${0%/*}
BIN=$(ls -t /usr/bin/clang-format* | head -1)
CONFIG=$SELF/bc.clang-format
SRCDIR=$(readlink -f "$SELF/../src")

find $SRCDIR -type f | \
    fgrep -vf <(\
        echo src/third_party
        echo src/benchmark
    ) | \
    xargs -I{} $SHELL -c "diff -u {} <($BIN --style=\"{ $(sed -e 's/#.*//' -e '/---/d' -e '/\.\.\./d' $CONFIG | tr $'\n' ,) }\" < {})"
