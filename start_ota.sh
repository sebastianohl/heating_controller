#!/bin/sh

mosquitto_pub -h $1 -u $2 -P $3 -r -t homie/heating-controller/system/update/set -m $4 -q 0
