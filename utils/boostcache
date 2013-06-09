#!/usr/bin/env bash

#
# Simple client in CLI
#
# Features:
# - rlwrap to add readline support
# - rlwrap to add autocompletion
#

set -e

HOST=${1:-"localhost"}
PORT=${2:-"9876"}

#
# TODO: avoid extra connection
#
COMMANDS=$(echo "COMMANDS" | nc -q1 $HOST $PORT | tail -n+2)
VERSION=$(echo "VERSION VERBOSE" \
            | nc -q1 $HOST $PORT \
            | tail -n+2 \
            | awk '{print $NF}')

# TODO: exit by Ctrl-D
cat - <<EOF
boostcache ${VERSION}

To exit from boostcache, hit Ctrl-C
EOF

rlwrap -m"> " \
       -S "boostcache> " \
       -f <(echo $COMMANDS) \
       nc $HOST $PORT
