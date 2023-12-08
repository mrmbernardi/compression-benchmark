#!/bin/bash
set -e
cd ${0%/*}
names=$(./build/compression-benchmark --names)
consts="${names//(lossless)/}"
consts="${consts// with /_}"
consts="${consts//\(/}"
consts="${consts//\)/}"
consts="${consts// /}"
consts="${consts^^}"
array=$(sed 's/.*/    &,/' <(echo "$consts"))
consts=$(sed 's/.*/& /' <(echo "$consts"))
names=$(sed 's/.*/ \"&\" /' <(echo "$names"))
paste -d'=' <(echo "$consts") <(echo "$names")
echo
echo 'AllMethods = ['
echo "$array"
echo ']'
