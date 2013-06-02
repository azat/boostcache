#!/usr/bin/env bash

set -e

SELF=${0%/*}
BOOSTCACHED=${1:-"$SELF/../.cmake/boostcached"}
BC_BENCHMARK=${2:-"$SELF/../src/benchmark/bc-benchmark"}
ONE_CONN_BENCHMARK=${3:-"$SELF/one_conn_awk_bench.sh"}
SOCKET=${4:-"$SELF/../.cmake/boostcached.sock"}

#
# Helper function
#

function indent()
{
    $1 | awk '{if ($0 == "") { exit; } print "\t" $0}'
}

function get_machine_info()
{
    echo "CPU info"

    if $( which lscpu &>/dev/null ); then
        indent "lscpu"
    else
        indent "cat /proc/cpuinfo"
    fi
    exit
}

# Start server, run benchmark, and kill server
#
# @param server
# @param benchmark
#
run_benchmark()
{
    echo "Starting server '$1'"
    eval "$1 &"
    SERVER_PID="$!"
    trap "echo Killing server; ps u $SERVER_PID &> /dev/null && kill $SERVER_PID" EXIT
    sleep 2
    echo "Starting benchmark '$2'"
    $2
    kill $SERVER_PID
}

#
# Some information
#
get_machine_info
# Check version, and existing of binary (see "set -e")
$BOOSTCACHED -V

#
# bc-benchmark
#
BC_BENCHMARK_OPTIONS=(
    ""
    "-r 100000000000" # Randomize keys
    "-s $SOCKET"
    "-s $SOCKET -r 100000000000"
)

for OPTIONS in "${BC_BENCHMARK_OPTIONS[@]}"; do
    #
    # Multiple clients
    #
    # Run bc-benchmark, with 3 workers
    # We have multiple connections, this must speedup server
    run_benchmark "$BOOSTCACHED -w3" "$BC_BENCHMARK $OPTIONS"
done

#
# One client
#
# Run one_connection, with 1 worker
# We have one connection, so we don't need in more than 1 worker.
run_benchmark "$BOOSTCACHED -w1" "$ONE_CONN_BENCHMARK"
