#!/bin/bash

rm *_miss.trace

python3 cache.py L1 $1
python3 cache.py L2 L1_miss.trace
python3 cache.py LLC L2_miss.trace
