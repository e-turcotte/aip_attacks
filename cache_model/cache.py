#!python3

import sys
import json

from treeplru import TreePLRUCache
from srrip import SRRIPCache
from drrip import DRRIPCache

INIT_STATE = True

LOG_FILE = ''
TRACE_FILE = ''

def perform_access(cache, access, interact):
    if access.lower() in ('q', 'quit'):
        return 1

    if access.split(' ')[0].lower() in ('f', 'file'):
        if interact:
            print(f"\n===Entering cache trace: {access.split(' ')[1]}===\n\n")
        with open(LOG_FILE, 'a') as f:
            f.write(f"\n===Entering cache trace: {access.split(' ')[1]}===\n\n\n")
        run_trace(access.split(' ')[1], cache)
        if interact:
            print(f"\n===Exiting cache trace: {access.split(' ')[1]}===\n\n")
            print(cache.print_state())
        with open(LOG_FILE, 'a') as f:
            f.write(f"\n===Exiting cache trace: {access.split(' ')[1]}===\n\n\n")
            f.write(cache.print_state() + '\n')
        return 0

    if access.lower() in ('i', 'interact'):
        if interact:
            print(f"\n===Entering interactive mode===\n\n")
        with open(LOG_FILE, 'a') as f:
            f.write(f"\n===Entering interactive mode===\n\n\n")
        run_interactive(cache)
        if interact:
            print(f"\n===Exiting interactive mode===\n\n")
            print(cache.print_state())
        with open(LOG_FILE, 'a') as f:
            f.write(f"\n===Exiting interactive mode===\n\n\n")
            f.write(cache.print_state() + '\n')
        return 0
    
    parts = access.split()
    if len(parts) != 2:
        return 0

    set_idx, tag = int(parts[0]), parts[1]
    hit = cache.access(set_idx, tag)
    if interact:
        print(f"Access set {set_idx}, tag {tag} -> {'HIT' if hit else 'MISS'}")
        print(cache.print_state())
    with open(LOG_FILE, 'a') as f:
        f.write(f"Access set {set_idx}, tag {tag} -> {'HIT' if hit else 'MISS'}\n")
        f.write(cache.print_state() + '\n')
    if not hit:
        with open(TRACE_FILE, 'a') as f:
            f.write(f"{set_idx} {tag}\n")

    return 0

def run_trace(filename, cache):
    with open(filename, 'r') as f:
        for line in f:
            #print(line)
            if perform_access(cache, line.strip(), False):
                break

def run_interactive(cache):
    while True:
        try:
            line = input("Next Access: ").strip()
        except EOFError:
            break
        
        if perform_access(cache, line, True):
            break

def run_cache(name, suffix, trace):
    global LOG_FILE 
    LOG_FILE = f"{name}{suffix}.log"
    global TRACE_FILE 
    TRACE_FILE = f"{name}{suffix}_miss.trace"

    cache_setup = dict()
    with open('cache_macros.json', 'r') as f:
        cache_setup = json.load(f)[name]

    match cache_setup['CACHE_RP'].lower():
        case 'treeplru':
            cache = TreePLRUCache(cache_setup['NUM_SETS'], cache_setup['ASSOC'], INIT_STATE)
        case 'srrip':
            cache = SRRIPCache(cache_setup['NUM_SETS'], cache_setup['ASSOC'], cache_setup['MAX_RRPV'], cache_setup['INSERTION_POS'], INIT_STATE)
        case 'drrip':
            cache = DRRIPCache(cache_setup['NUM_SETS'], cache_setup['ASSOC'], cache_setup['MAX_RRPV'], cache_setup['INSERTION_POS'], cache_setup['NUM_LEADER_SETS'], cache_setup['PSEL_BITS'], cache_setup['BRRIP_PROB'], INIT_STATE)
        case _:
            print('ERROR: Not a valid $RP')

    if trace:
        run_trace(trace, cache)
    else:
        cache.print_state()
        run_interactive(cache)



if __name__ == "__main__":
    run_cache(sys.argv[1], '', sys.argv[2] if len(sys.argv) == 3 else None)
