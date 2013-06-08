#!/usr/bin/env bash

#
# Build graphs using gnuplot
#

set -e

SELF=${0%/*}
if [ ! ${SELF:0:1} = "/" ]; then
    SELF="$PWD/$SELF/"
fi
PLOTS_ROOT="$SELF/plots"
BOOSTCACHED=${1:-"$SELF/../.cmake/boostcached"}
BC_BENCHMARK=${2:-"$SELF/../src/benchmark/bc-benchmark"}
SOCKET=${4:-"$SELF/../.cmake/boostcached.sock"}
WORKERS_CONF=(1 2 4 5 10)
WORKERS_CONF_LENGTH=${#WORKERS_CONF[@]}

mkdir -p "$PLOTS_ROOT"

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
    rm -f "$PLOTS_ROOT"/*.plot.data
    rm -f "$PLOTS_ROOT"/*.plot
    rm -f "$PLOTS_ROOT"/*.plot.png

    for CMD in HSET HGET HDEL ATSET ATGET ATDEL; do
        cat > "$PLOTS_ROOT/$CMD.plot" <<EOL
set title "${CMD}"
set xlabel "Clients"
set ylabel "Seconds"

set terminal pngcairo transparent enhanced font "arial,10" fontscale 1.0 size 500, 350
set output "$PLOTS_ROOT/${CMD}.plot.png"

plot \\
EOL

        for (( i=0; i < $WORKERS_CONF_LENGTH; ++i )); do
            WORKERS=${WORKERS_CONF[$i]}

            LINEWRAP=""
            if [[ $WORKERS_CONF_LENGTH -gt $((i + 1)) ]]; then
                LINEWRAP=", \\"
            fi

            cat >> "$PLOTS_ROOT/$CMD.plot" <<EOL
    "$PLOTS_ROOT/${CMD}.workers_${WORKERS}.plot.data" \
        title "${WORKERS} workers" \
        with linespoint${LINEWRAP}
EOL
        done
    done
}

# @param clients
# @param workers
#
build_graph()
{
    local CLIENTS=$1
    local WORKERS=$2

    while read LINE; do
        CMD=$(echo $LINE | awk '{print $1}' | tr -d :)
        TIME=$(echo $LINE | awk '{print $2}' | sed -r 's/[^0-9.,]//g')

        if [ -e $CMD ]; then
            continue
        fi

        printf "%d %.4f\n" $CLIENTS $TIME >> "$PLOTS_ROOT/${CMD}.workers_${WORKERS}.plot.data"
    done
}

prepare_graph
for CLIENTS in 1 5 10 20 30 40 50 100; do
    for (( i=0; i < $WORKERS_CONF_LENGTH; ++i )); do
        WORKERS=${WORKERS_CONF[$i]}

        run_benchmark "$BOOSTCACHED -w $WORKERS -s $SOCKET" "$BC_BENCHMARK -s $SOCKET -q -c $CLIENTS" | \
            awk '{printf "%s %s\n", $1, $2}' | build_graph $CLIENTS $WORKERS
    done
done

for PLOT in "$PLOTS_ROOT"/*.plot; do
    gnuplot -p "$PLOT"
done

# Join all plots into one picture using imagick
montage "$PLOTS_ROOT"/*.plot.png \
    -shadow -background none -geometry +0+0 \
    "$PLOTS_ROOT"/boostcache.png

