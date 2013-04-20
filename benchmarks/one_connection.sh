#!/usr/bin/env bash

set -e

if ! $( pidof boostcached &> /dev/null ); then
    echo "First start boostcached"
    exit 1
fi

#PORT=9876
#HOST=localhost
LIMIT=1000000
TMP_FILE="/tmp/test-boostcached-commands-$RANDOM.txt"

trap 'echo Removing temporary data; rm -f $TMP_FILE' EXIT

function run_test_client()
{
    echo "Sending data"

    START=$(date +%s)
    COUNT=$(gawk 'BEGIN {
        serverConnection = "/inet/tcp/0/localhost/9876"
        count=0
    }
    {
        # write to server
        print $0 |& serverConnection
        ++count
        # read from server
        serverConnection |& getline
        # We do not need to print output test data
        # print $0
    }
    END {
        close(service)
        print count
    }' $1)
    END=$(date +%s)
    ESTIMATE=$((END - START))
    # Queries per second
    QPS=$((COUNT / ESTIMATE))

    echo "Estimate $ESTIMATE sec ($QPS qps)"
}

echo "Preparing data for HSET ..."
for ((i=1; i<=$LIMIT; i++)); do
    echo "HSET ${i}_key ${i}_value" >> $TMP_FILE
done
run_test_client $TMP_FILE

rm $TMP_FILE
echo "Preparing data for HGET ..."
for ((i=1; i<=$LIMIT; i++)); do
    echo "HGET ${i}_key" >> $TMP_FILE
done
run_test_client $TMP_FILE
