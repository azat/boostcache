#!/usr/bin/env bash

# XXX: More tests

set -e

SELF=${0%/*}
if [ ! ${SELF:0:1} = "/" ]; then
    SELF="$PWD/$SELF/"
fi
BOOSTCACHED=${1:-"$SELF/../.cmake/boostcached"}

startServer()
{
    $BOOSTCACHED -w2 &
    SERVER_PID="$!"
    trap "echo Killing server; ps u $SERVER_PID &> /dev/null && kill $SERVER_PID" EXIT

    sleep 2
}

startServer

$SELF/test-js.sh
