#!/bin/bash

rm *_miss.trace

python3 cache.py L1 $1 > L1.log
python3 cache.py L2 L1_miss.trace > L2.log
python3 cache.py LLC L2_miss.trace > LLC.log
