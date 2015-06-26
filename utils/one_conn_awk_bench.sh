#!/usr/bin/env bash

set -e

PORT=9876
HOST=localhost
LIMIT=100000

if ! $( netstat -an | egrep -q "tcp.*:$PORT.*LISTEN" ); then
    echo "First start boostcached (I can't find it at $PORT port)"
    exit 1
fi

function run_test_client()
{
    local FMT_STRING=$1

    echo "Testing '$FMT_STRING'"

    START=$(date +%s)
    COUNT=$(gawk "
    BEGIN {
        serverConnection = \"/inet/tcp/0/$HOST/$PORT\"
        count=0
        limit=$LIMIT
        fmt_string=\"$FMT_STRING\r\n\"

        for (i = 0; i < limit; ++i) {
            # progress
            if ((i % 1000) == 0) {
                printf(\"Processed requests: %.0f%% (%i of %i)\r\",
                       ((i / limit) * 100), i, limit) > \"/dev/stderr\"
            }
            printf fmt_string, i, i |& serverConnection
            ++count
            serverConnection |& getline
            # We do not need to print output test data
            # print \$0
        }

        # Flush progress bar
        print \"\" > \"/dev/stderr\"

        close(serverConnection)
        print count
    }
    ")
    END=$(date +%s)
    ESTIMATE=$((END - START))
    # Queries per second
    QPS=$((COUNT / ESTIMATE))

    echo "Estimate $ESTIMATE sec ($QPS qps)"
}

# Hash table
run_test_client "HSET %i_key %i_value"
run_test_client "HGET %i_key"
run_test_client "HDEL %i_key"

# Avl tree
run_test_client "ATSET %i_key %i_value"
run_test_client "ATGET %i_key"
run_test_client "ATDEL %i_key"

# Other
run_test_client "PING"
run_test_client "NOT_EXISTED_COMMAND"
