#! /usr/bin/python3

import sys
import random

# === Cache Configuration Macros ===
NUM_SETS = 16
ASSOC = 4
MAX_RRPV = 3
NUM_LEADER_SETS = 2     # per policy
PSEL_BITS = 5
BRRIP_PROB = 32         # average 1/X inserts as SRRIP

class DRRIPCache:
    def __init__(self, num_sets, assoc, max_rrpv=3, num_leader_sets=2, psel_bits=10, brrip_prob=32):
        self.num_sets = num_sets
        self.assoc = assoc
        self.max_rrpv = max_rrpv
        self.sets = [[{'tag': None, 'rrpv': max_rrpv} for _ in range(assoc)] for _ in range(num_sets)]

        self.num_leader_sets = num_leader_sets
        self.psel_max = (1 << psel_bits) - 1
        self.psel = self.psel_max // 2
        self.brrip_prob = brrip_prob

        step = num_sets // (2 * num_leader_sets)
        self.srrip_leaders = [(i * step * 2) % num_sets for i in range(num_leader_sets)]
        self.brrip_leaders = [(i * step * 2 + step) % num_sets for i in range(num_leader_sets)]

    def _policy_for_set(self, set_idx):
        if set_idx in self.srrip_leaders:
            return "SRRIP"
        if set_idx in self.brrip_leaders:
            return "BRRIP"
        return "SRRIP" if self.psel >= self.psel_max // 2 else "BRRIP"

    def access(self, set_idx, tag):
        policy = self._policy_for_set(set_idx)
        cache_set = self.sets[set_idx]

        for line in cache_set:
            if line['tag'] == tag:
                line['rrpv'] = 0
                if set_idx in self.srrip_leaders:
                    self.psel = min(self.psel + 1, self.psel_max)
                elif set_idx in self.brrip_leaders:
                    self.psel = max(self.psel - 1, 0)
                return True

        while True:
            for line in cache_set:
                if line['rrpv'] == self.max_rrpv:
                    line['tag'] = tag
                    if policy == "SRRIP":
                        line['rrpv'] = self.max_rrpv - 1
                    else:
                        if random.randint(1, self.brrip_prob) == 1:
                            line['rrpv'] = self.max_rrpv - 1
                        else:
                            line['rrpv'] = self.max_rrpv
                    return False
            for line in cache_set:
                line['rrpv'] += 1

    def print_state(self):
        for i, cache_set in enumerate(self.sets):
            if i in self.srrip_leaders:
                label = "S"
            elif i in self.brrip_leaders:
                label = "B"
            else:
                label = "f"
            line_str = " | ".join(f"{line['tag'] if line['tag'] else 'empty'}({line['rrpv']})" for line in cache_set)
            print(f"Set {i:2d} {label}: {line_str}")
        follower_policy = "SRRIP" if self.psel >= self.psel_max // 2 else "BRRIP"
        print(f"PSEL={self.psel}, Follower policy={follower_policy}\n")

def run_trace(filename, cache):
    with open(filename) as f:
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
        parts = line.split()
        if len(parts) != 2:
            continue
        set_idx, tag = int(parts[0]), parts[1]
        hit = cache.access(set_idx, tag)
        print(f"Access set {set_idx}, tag {tag} -> {'HIT' if hit else 'MISS'}")
        cache.print_state()

if __name__ == "__main__":
    cache = DRRIPCache(NUM_SETS, ASSOC, MAX_RRPV, NUM_LEADER_SETS, PSEL_BITS, BRRIP_PROB)

    if len(sys.argv) == 2:
        run_trace(sys.argv[1], cache)
    run_interactive(cache)

