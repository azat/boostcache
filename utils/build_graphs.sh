#!/usr/bin/env bash

#
# Build graphs using gnuplot
#

set -e

SELF=${0%/*}
if [ ! ${SELF:0:1} = "/" ]; then
    SELF="$PWD/$SELF/"
fi
BOOSTCACHED=${1:-"$SELF/../.cmake/boostcached"}
BC_BENCHMARK=${2:-"$SELF/../src/benchmark/bc-benchmark"}
SOCKET=${4:-"$SELF/../.cmake/boostcached.sock"}

# Start server, run benchmark, and kill server
#
# @param server
# @param benchmark
#
run_benchmark()
{
    eval "$1 &"
    SERVER_PID="$!"
    trap "ps u $SERVER_PID &> /dev/null && kill $SERVER_PID" EXIT
    sleep 0.1
    $2
    kill $SERVER_PID
}

prepare_graph()
{
    rm -f *.plot.data
    rm -f *.plot
    rm -f *.plot.png

    for CMD in HSET HGET HDEL ATSET ATGET ATDEL; do
        cat > $SELF/$CMD.plot <<EOL
set title "${CMD}"
set xlabel "Clients"
set ylabel "Seconds"

set terminal pngcairo  transparent enhanced font "arial,10" fontscale 1.0 size 500, 350
set output "${CMD}.plot.png"

# with linespoint
plot [] [] "${CMD}.plot.data" using 1:2 with linespoint
EOL
    done
}

# @param clients
#
build_graph()
{
    local CLIENTS=$1

    while read LINE; do
        CMD=$(echo $LINE | awk '{print $1}' | tr -d :)
        TIME=$(echo $LINE | awk '{print $2}' | sed -r 's/[^0-9.,]//g')

        if [ -e $CMD ]; then
            continue
        fi

        printf "%d %.4f\n" $CLIENTS $TIME >> $CMD.plot.data
    done
}

prepare_graph
# TODO: workers loop
for CLIENTS in 1 5 10 20 30 40 50 100 200; do
    run_benchmark "$BOOSTCACHED -s $SOCKET" "$BC_BENCHMARK -s $SOCKET -q -c $CLIENTS" | \
        awk '{printf "%s %s\n", $1, $2}' | build_graph $CLIENTS $WORKERS
done

for PLOT in *.plot; do
    gnuplot -p "$PLOT"
done