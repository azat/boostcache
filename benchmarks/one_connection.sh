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
    gawk 'BEGIN {
        serverConnection = "/inet4/tcp/0/localhost/9876"
    }
    {
        # write to server
        print $0 |& serverConnection
        # read from server
        serverConnection |& getline
        # We do not need to print output test data
        # print $0
    }
    END {
        close(service)
    }' $1
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
