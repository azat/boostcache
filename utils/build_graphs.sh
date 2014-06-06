#!/usr/bin/env bash

#
# Build graphs using gnuplot
#

set -e

SELF=${0%/*}
if [ ! ${SELF:0:1} = "/" ]; then
    SELF="$PWD/$SELF/"
fi
PLOTS_ROOT="$SELF/plots-$(git log --format='%h-%f' -n1)"
# Available next:
# - png
# - dumb
FORMAT="png"
BOOSTCACHED="$SELF/../.cmake/boostcached"
BC_BENCHMARK="$SELF/../src/benchmark/bc-benchmark"
SOCKET="$SELF/../.cmake/boostcached.sock"
WORKERS_CONF=(1 2 4 10)
WORKERS_CONF_LENGTH=${#WORKERS_CONF[@]}
COMMANDS="HSET HGET HDEL ATSET ATGET ATDEL PING NOT_EXISTED_COMMAND"
RANDOM_ITERATIONS=4
CLIENTS_CONF="1 5 10 20 30 40 50 100"

mkdir -p "$PLOTS_ROOT"

function print_help()
{
    echo "Usage: $0 [ OPTS ]"
    echo
    echo " -f    - set format (png, dumb)" 2>&1
    echo " -b    - boostcached binary" 2>&1
    echo " -B    - bc-benchmark binray" 2>&1
    echo " -s    - default socket for boostcached" 2>&1
    echo " -w    - workers" 2>&1
    echo " -c    - commands to benchmarking" 2>&1
    echo " -r    - number of random iterations" 2>&1
    echo " -C    - clients" 2>&1
    exit 1
}

function parse_options()
{
    while getopts "h?f:b:B:s:w:c:r:C:" o
        do case "$o" in
            h)  print_help;;
            f)  FORMAT="$OPTARG";;
            b)  BOOSTCACHED="$OPTARG";;
            B)  BC_BENCHMARK="$OPTARG";;
            s)  SOCKET="$OPTARG";;
            w)  WORKERS_CONF=($OPTARG);;
            c)  COMMANDS="$OPTARG";;
            r)  RANDOM_ITERATIONS="$OPTARG";;
            C)  CLIENTS_CONF="$OPTARG";;
        esac
    done

    WORKERS_CONF_LENGTH=${#WORKERS_CONF[@]}
}

# Start server, run benchmark, and kill server
#
# @param server
# @param benchmark
#
function run_benchmark()
{
    eval "$1 >/dev/null &"
    SERVER_PID="$!"
    trap "ps u $SERVER_PID &> /dev/null && kill $SERVER_PID" EXIT
    sleep 0.1
    $2 -t ${COMMANDS// /,}
    kill -9 $SERVER_PID
}

function prepare_graph()
{
    rm -f "$PLOTS_ROOT"/*.plot.data
    rm -f "$PLOTS_ROOT"/*.plot
    rm -f "$PLOTS_ROOT"/*.plot.png

    for CMD in $COMMANDS; do
        TERMINAL_OPTIONS=""
        if [ $FORMAT = "png" ]; then
            TERMINAL_OPTIONS=$(cat <<EOL
set terminal pngcairo transparent enhanced size 760,480 font 'Gill Sans,9' rounded dashed
set output "$PLOTS_ROOT/${CMD}.plot.png"
EOL
            )
        elif [ $FORMAT = "dumb" ]; then
            TERMINAL_OPTIONS="set terminal dumb"
        fi

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

$TERMINAL_OPTIONS

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
function build_graph_data()
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

function plot_graphs()
{
    for PLOT in "$PLOTS_ROOT"/*.plot; do
        gnuplot -p "$PLOT"
    done

    if [ $FORMAT = "png" ]; then
        # Join all plots into one picture using imagick
        montage "$PLOTS_ROOT"/*.plot.png \
            -background none -geometry +0+0 \
            "$PLOTS_ROOT"/boostcache.png
    fi
}

function build_graphs()
{
    RESULTS_TEMP=$(mktemp)

    for CLIENTS in $CLIENTS_CONF; do
        for (( i=0; i < $WORKERS_CONF_LENGTH; ++i )); do
            WORKERS=${WORKERS_CONF[$i]}

            # Try to avoid randomization
            for _RAND in $(seq 1 $RANDOM_ITERATIONS); do
                run_benchmark "$BOOSTCACHED --unixsocket $SOCKET" "$BC_BENCHMARK -s $SOCKET -q -c $CLIENTS" | \
                    # emulate carriage return
                    sed 's/^.*\r//' | \
                    awk '{printf "%s %s\n", $1, $2}' >> \
                    $RESULTS_TEMP
            done

            unset -v AVG
            declare -A AVG

            while read LINE; do
                CMD=$(echo $LINE | awk '{print $1}' | tr -d :)
                TIME=$(echo $LINE | awk '{print $2}' | sed -r 's/[^0-9.,]//g')

                if [ -e $CMD ]; then
                    continue
                fi

                if [ -e ${AVG[$CMD]} ]; then
                    AVG[$CMD]=0
                fi
                # Bash don't have float point arithemits [?]
                AVG[$CMD]=$(echo "${AVG[$CMD]}+$TIME" | bc -l)
            done < $RESULTS_TEMP

            echo -n > $RESULTS_TEMP

            for CMD in "${!AVG[@]}"; do
                printf "%s %.4f\n" $CMD $(echo "${AVG[$CMD]}/$RANDOM_ITERATIONS" | bc -l) | \
                       build_graph_data $CLIENTS $WORKERS
            done
        done
    done

    rm $RESULTS_TEMP
}

parse_options "$@"
prepare_graph
build_graphs
plot_graphs
