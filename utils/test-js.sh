#!/usr/bin/env bash

#
# Do some checks for jsvm.
# But firstly you must start server.
#

set -e

timeout=10000
host=localhost
port=9876

IFS='' jsForEach=$(cat <<EOF
(function(key, value) {
    console.log("Key: " + key + ", value: " + value);
})
EOF
)
IFS='' jsThrow=$(cat <<EOF
(function(key, value) {
    throw ("Checking throw handling: " + key + " " + value);
})
EOF
)

function send() { nc -q$timeout $host $port; }
function checkOkResponse() { grep -q $'^+OK\r$'; }
function checkTrueResponse() { grep -q $'^:1\r$'; }
function checkErrorResponse() { grep -q $'^-ERR\r$'; }
# XXX: add sendInlineRequest
function sendBulkRequest()
{
    local argc=$#
    local crlf=$'\r\n'
    local request='*'$argc$crlf

    for arg; do
        local argLen=${#arg}
        request+='$'$argLen$crlf$arg$crlf
    done

    echo "$request" | send
}

# Fill data
for i in {1..1000}; do
    sendBulkRequest HSET foo$i bar$i | checkOkResponse
done
sendBulkRequest HFOR "$jsForEach" | checkTrueResponse
sendBulkRequest HFOR "$jsThrow" | checkErrorResponse
