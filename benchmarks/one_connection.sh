#!/usr/bin/env bash

set -e

if ! $( pidof boostcached &> /dev/null ); then
    echo "First start boostcached"
    exit 1
fi

PORT=9876
HOST=localhost
LIMIT=1000000

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
            printf fmt_string, i, i |& serverConnection
            ++count
            serverConnection |& getline
            # We do not need to print output test data
            # print \$0
        }

        close(service)
        print count
    }
    ")
    END=$(date +%s)
    ESTIMATE=$((END - START))
    # Queries per second
    QPS=$((COUNT / ESTIMATE))

    echo "Estimate $ESTIMATE sec ($QPS qps)"
}

run_test_client "HSET %i_key %i_value"
run_test_client "HGET %i_key"
