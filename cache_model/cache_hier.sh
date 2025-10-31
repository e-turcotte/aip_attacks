#!/bin/bash

rm *_miss.trace

./treeplru.py $1
./srrip.py tplru_miss.trace
./drrip.py srrip_miss.trace
