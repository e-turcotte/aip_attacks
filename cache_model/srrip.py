#! /usr/bin/python3

import sys
import random

# === Cache Configuration Macros ===
NUM_SETS = 16
ASSOC = 4
MAX_RRPV = 3
INIT_STATE = True

class SRRIPCache:
    def __init__(self):
        self.num_sets = NUM_SETS
        self.assoc = ASSOC
        self.max_rrpv = MAX_RRPV
        if INIT_STATE:
            self.sets = [[{'tag': '~init', 'rrpv': random.randint(0,MAX_RRPV)} for _ in range(ASSOC)] for _ in range(NUM_SETS)]
        else:
            self.sets = [[{'tag': '~empty', 'rrpv': MAX_RRPV} for _ in range(ASSOC)] for _ in range(NUM_SETS)]

    def access(self, set_idx, tag):
        cache_set = self.sets[set_idx]

        for line in cache_set:
            if line['tag'] == tag:
                line['rrpv'] = 0
                return True

        with open('srrip_miss.trace', 'a') as f:
            f.write(f"{set_idx} {tag}\n")

        while True:
            for line in cache_set:
                if line['rrpv'] == self.max_rrpv:
                    line['tag'] = tag
                    line['rrpv'] = self.max_rrpv - 1
                    return False
            for line in cache_set:
                line['rrpv'] += 1

    def print_state(self):
        for i, cache_set in enumerate(self.sets):
            line_str = " | ".join(f"{line['tag']}({line['rrpv']})" for line in cache_set)
            print(f"Set {i:2d}: {line_str}")
        print(f"")

def run_trace(filename, cache):
    with open(filename, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) != 2:
                continue
            set_idx, tag = int(parts[0]), parts[1]
            hit = cache.access(set_idx, tag)
            print(f"Access set {set_idx}, tag {tag} -> {'HIT' if hit else 'MISS'}")
            cache.print_state()

def run_interactive(cache):
    while True:
        try:
            line = input("Next Access: ").strip()
        except EOFError:
            break
        if line.lower() in ('q', 'quit'):
            break
        if line.lower().split(' ')[0] in ('f', 'file'):
            print(f"\n===Entering cache trace===\n\n")
            run_trace(line.split(' ')[1], cache)
            print(f"\n===Exiting cache trace, Entering interactive mode===\n\n")
            cache.print_state()
            continue
        parts = line.split()
        if len(parts) != 2:
            continue
        set_idx, tag = int(parts[0]), parts[1]
        hit = cache.access(set_idx, tag)
        print(f"Access set {set_idx}, tag {tag} -> {'HIT' if hit else 'MISS'}")
        cache.print_state()

if __name__ == "__main__":
    cache = SRRIPCache()

    if len(sys.argv) == 2:
        run_trace(sys.argv[1], cache)
    #    print(f"\n===Exiting cache trace, Entering interactive mode===\n\n")
    #cache.print_state()
    #run_interactive(cache)

