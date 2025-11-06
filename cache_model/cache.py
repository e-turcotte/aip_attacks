#!python3

import sys
import json

from treeplru import TreePLRUCache
from srrip import SRRIPCache
from drrip import DRRIPCache

INIT_STATE = True
CACHE_NAME = ''

def perform_access(cache, access):
    if access.lower() in ('q', 'quit'):
        return 1

    if access.split(' ')[0].lower() in ('f', 'file'):
        print(f"\n===Entering cache trace: {access.split(' ')[1]}===\n\n")
        run_trace(access.split(' ')[1], cache)
        print(f"\n===Exiting cache trace: {access.split(' ')[1]}===\n\n")
        cache.print_state()
        return 0

    if access.lower() in ('i', 'interact'):
        print(f"\n===Entering interactive mode===\n\n")
        run_interactive(cache)
        print(f"\n===Exiting interactive mode===\n\n")
        cache.print_state()
        return 0

    parts = access.split()
    if len(parts) != 2:
        return 0

    set_idx, tag = int(parts[0]), parts[1]
    hit = cache.access(set_idx, tag)
    print(f"Access set {set_idx}, tag {tag} -> {'HIT' if hit else 'MISS'}")
    if not hit:
        with open(f"{CACHE_NAME}_miss.trace", 'a') as f:
            f.write(f"{set_idx} {tag}\n")

    cache.print_state()
    return 0

def run_trace(filename, cache):
    with open(filename, 'r') as f:
        for line in f:
            if perform_access(cache, line.strip()):
                break

def run_interactive(cache):
    while True:
        try:
            line = input("Next Access: ").strip()
        except EOFError:
            break
        
        if perform_access(cache, line):
            break

if __name__ == "__main__":
    CACHE_NAME = sys.argv[1]

    cache_setup = dict()
    with open('cache_macros.json', 'r') as f:
        cache_setup = json.load(f)[CACHE_NAME]

    match cache_setup['CACHE_RP'].lower():
        case 'treeplru':
            cache = TreePLRUCache(cache_setup['NUM_SETS'], cache_setup['ASSOC'], INIT_STATE)
        case 'srrip':
            cache = SRRIPCache(cache_setup['NUM_SETS'], cache_setup['ASSOC'], cache_setup['MAX_RRPV'], cache_setup['INSERTION_POS'], INIT_STATE)
        case 'drrip':
            cache = DRRIPCache(cache_setup['NUM_SETS'], cache_setup['ASSOC'], cache_setup['MAX_RRPV'], cache_setup['INSERTION_POS'], cache_setup['NUM_LEADER_SETS'], cache_setup['PSEL_BITS'], cache_setup['BRRIP_PROB'], INIT_STATE)
        case _:
            print('ERROR: Not a valid $RP')

    if len(sys.argv) == 3:
        run_trace(sys.argv[2], cache)
    else:
        cache.print_state()
        run_interactive(cache)

