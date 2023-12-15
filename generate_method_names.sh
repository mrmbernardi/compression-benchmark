#!/bin/bash
set -e
cd ${0%/*}
output=$(./build/compression-benchmark --names)

names=$(cut -d: -f1 <(echo "$output"))
dtypes=$(cut -d: -f2 <(echo "$output") | tr -d ' ' | sed 's/,/\", \"/; s/.*/\["&\"]/')
consts="${names//(lossless)/}"
consts="${consts// with /_}"
consts="${consts//\(/}"
consts="${consts//\)/}"
consts="${consts// /}"
consts="${consts^^}"
array=$(sed 's/.*/    &,/' <(echo "$consts"))
consts=$(sed 's/.*/& /' <(echo "$consts"))
methods=$(sed 's/.*/ Method\(\"&\"/' <(echo "$names"))
methods=$(paste -d', ' <(echo "$methods") <(echo "$dtypes" | sed 's/.*/ &\)/'))
paste -d'=' <(echo "$consts") <(echo "$methods")
echo
echo 'AllMethods = ['
echo "$array"
echo ']'
