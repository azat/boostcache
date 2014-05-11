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
    return value + "_updated";
})
EOF
)
IFS='' jsThrow=$(cat <<EOF
(function(key, value) {
    throw ("Checking throw handling: " + key + " " + value);
})
EOF
)
IFS='' jsCheckUpdatedKeys=$(cat <<EOF
(function(key, value) {
    if (value.indexOf("_updated") == -1) {
        throw ("Key " + key + " not updated");
    }
})
EOF
)

function send()
{
    realnc=$(readlink -f $(which nc))
    if [[ "$realnc" =~ ".traditional" ]]; then
        nc -q$timeout -w$timeout $host $port
    else # It's likely to be openbsd version of nc
        nc -w$timeout $host $port
    fi
}
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
sendBulkRequest HFOR "$jsCheckUpdatedKeys" | checkTrueResponse
