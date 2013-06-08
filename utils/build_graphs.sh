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
WORKERS_CONF=(1 2 4 10)
WORKERS_CONF_LENGTH=${#WORKERS_CONF[@]}
COMMANDS="HSET HGET HDEL ATSET ATGET ATDEL PING NOT_EXISTED_COMMAND"

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

    for CMD in $COMMANDS; do
        cat > "$PLOTS_ROOT/$CMD.plot" <<EOL
set title "${CMD}"
set xlabel "Clients"
set ylabel "Requests per second"

# Line style for axes
set style line 80 lt 0
set style line 80 lt rgb "#808080"

# Line style for grid
set style line 81 lt 3  # dashed
set style line 81 lt rgb "#808080" lw 0.5 # grey

set grid back linestyle 81
set border 3 back linestyle 80
set xtics nomirror
set ytics nomirror

set style line 1 lt 1
set style line 2 lt 1
set style line 3 lt 1
set style line 4 lt 1
set style line 1 lt rgb "#A00000" lw 2 pt 7
set style line 2 lt rgb "#00A000" lw 2 pt 9
set style line 3 lt rgb "#5060D0" lw 2 pt 5
set style line 4 lt rgb "#F25900" lw 2 pt 13

set terminal pngcairo transparent enhanced size 760,480 font "Gill Sans,9" rounded dashed
set output "$PLOTS_ROOT/${CMD}.plot.png"

plot \\
EOL

        for (( i=0; i < $WORKERS_CONF_LENGTH; ++i )); do
            WORKERS=${WORKERS_CONF[$i]}
            LINESTYLE=$((i + 1))

            LINEWRAP=""
            if [[ $WORKERS_CONF_LENGTH -gt $((i + 1)) ]]; then
                LINEWRAP=", \\"
            fi

            cat >> "$PLOTS_ROOT/$CMD.plot" <<-EOL
    "$PLOTS_ROOT/${CMD}.workers_${WORKERS}.plot.data" \
        title "${WORKERS} workers" \
        ls ${LINESTYLE} \
        with linespoint, \
        '' using 1:2:(sprintf("%.0fK", \$2/1000)) with labels offset 0.5,0.5 notitle \
        ${LINEWRAP}
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
            # emulate carriage return
            sed 's/^.*\r//' | \
            awk '{printf "%s %s\n", $1, $2}' | build_graph $CLIENTS $WORKERS
    done
done

for PLOT in "$PLOTS_ROOT"/*.plot; do
    gnuplot -p "$PLOT"
done

# Join all plots into one picture using imagick
montage "$PLOTS_ROOT"/*.plot.png \
    -background none -geometry +0+0 \
    "$PLOTS_ROOT"/boostcache.png

