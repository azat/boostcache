#!/usr/bin/env bash

#
# Simple client in CLI
#
# Features:
# - rlwrap to add readline support
# - rlwrap to add autocompletion
#

HOST=${1:-"localhost"}
PORT=${2:-"9876"}

COMMANDS="HGET HSET HDEL
          ATGET ATSET ATDEL"

rlwrap -f <(echo $COMMANDS) nc $HOST $PORT
