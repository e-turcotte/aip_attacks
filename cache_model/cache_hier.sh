#!/bin/bash

rm *_miss.trace

./treeplru.py $1 > L1.log
./srrip.py tplru_miss.trace > L2.log
./drrip.py srrip_miss.trace > LLC.log
