#!/usr/bin/env bash

set -e

if ! $( pidof boostcached &> /dev/null ); then
    echo "First start boostcached"
    exit 1
fi

INPUT_FIFO="/tmp/test-boostcached-input.$RANDOM"
OUTPUT_FIFO="/tmp/test-boostcached-output.$RANDOM"
PORT=9876
HOST=localhost
#LIMIT=10000000
LIMIT=10

mkfifo "$INPUT_FIFO"
mkfifo "$OUTPUT_FIFO"

cat "$INPUT_FIFO" | tee in.log | nc $HOST $PORT | tee out.log > $OUTPUT_FIFO &
PID="$!"
trap 'echo Killing nc, and removing tmp pipe; kill $PID; rm -f $INPUT_FIFO $OUTPUT_FIFO' EXIT

function write_read()
{
    echo $* >> $INPUT_FIFO
    timeout 10s head -n1 $OUTPUT_FIFO
}

echo "Testing HSET ..."
for ((i=1; i<=$LIMIT; i++)); do
    write_read "HSET ${i}_key ${i}_value"
done

echo "Testing HGET ..."
for ((i=1; i<=$LIMIT; i++)); do
    write_read "HGET ${i}_key" >> "$INPUT_FIFO"
done
