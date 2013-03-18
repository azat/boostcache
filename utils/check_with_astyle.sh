#!/usr/bin/env bash

#
# TODO: add more options
#

DIRNAME=$(which dirname)

set -e

if [ ! "$DIRNAME" ]; then
    echo "dirname is not available" >&2
    exit 1
fi

SELF=$($DIRNAME "$0")
BIN=$(which astyle)
CONFIG=$(cat "$SELF/boostcache_astyle")
SRCDIR=$(readlink -f "$SELF/../src")

set +e
if ! $($BIN --version 2>&1 | fgrep -q "(with some patches from https://github.com/azat/astyle)"); then
    echo "astyle version didn't support some features." >&2
    echo "See at https://github.com/azat/astyle" >&2
    exit 2
fi
set -e

# TODO: enhanced file "iterator"
for FILE in $(find $SRCDIR -type f | fgrep -v src/third_party/); do
    $BIN --dry-run $CONFIG "$FILE"
done
