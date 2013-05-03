#!/usr/bin/env bash

set -e

SELF=${0%/*}
BOOSTCACHED=${1:-"$SELF/../.cmake/boostcached"}
BC_BENCHMARK=${1:-"$SELF/../src/benchmark/bc-benchmark"}
ONE_CONN_BENCHMARK=${1:-"$SELF/one_conn_awk_bench.sh"}

#
# Helper function
#
# Start server, run test, and kill server
#
# @param server
# @param test
#
run_test()
{
    echo "Starting server '$1'"
    eval "$1 &"
    SERVER_PID="$!"
    trap "echo Killing server; ps u $SERVER_PID &> /dev/null && kill $SERVER_PID" EXIT
    sleep 2
    echo "Starting test '$2'"
    $2
    kill $SERVER_PID
}

# Check version, and existing of binary (see "set -e")
$BOOSTCACHED -V

#
# One client
#
# Run one_connection, with 1 worker
# We have one connection, so we don't need in more than 1 worker.
run_test "$BOOSTCACHED -w1" "$ONE_CONN_BENCHMARK"

#
# Multiple clients
#
# Run bc-benchmark, with 3 workers
# We have multiple connections, this must speedup server
run_test "$BOOSTCACHED -w3" "$BC_BENCHMARK"

#
# Run with random keys
#
# Run bc-benchmark, with 3 workers
# We have multiple connections, this must speedup server
run_test "$BOOSTCACHED -w3" "$BC_BENCHMARK -r 100000000000"