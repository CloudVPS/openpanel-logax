#!/bin/bash
../logsplit -b 10 -r -i 180 -m 50000 -s  ./out ./out < ./data/test.input
echo "The ./out diretory should contain splitted logfiles now and a YYYYMM-badwidth.log file"
