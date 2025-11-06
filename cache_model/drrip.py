#! /usr/bin/python3

import random

# === Cache Configuration Macros ===
# NUM_SETS
# ASSOC
# MAX_RRPV
# INSERTION_POS
# NUM_LEADER_SETS -- per policy
# PSEL_BITS
# BRRIP_PROB -- average 1/X inserts as SRRIP

class DRRIPCache:
    def __init__(self, NUM_SETS, ASSOC, MAX_RRPV, INSERTION_POS, NUM_LEADER_SETS, PSEL_BITS, BRRIP_PROB, INIT_STATE):
        self.num_sets = NUM_SETS
        self.assoc = ASSOC
        self.max_rrpv = MAX_RRPV
        self.psel_max = (1 << PSEL_BITS) - 1
        self.brrip_prob = BRRIP_PROB
        
        if INIT_STATE:
            self.sets = [[{'tag': '~init', 'rrpv': random.randint(0,MAX_RRPV)} for _ in range(ASSOC)] for _ in range(NUM_SETS)]
            self.psel = random.randint(0,self.psel_max)
        else:
            self.sets = [[{'tag': '~empty', 'rrpv': MAX_RRPV} for _ in range(ASSOC)] for _ in range(NUM_SETS)]
            self.psel = self.psel_max // 2

        self.num_leader_sets = NUM_LEADER_SETS

        step = NUM_SETS // (2 * NUM_LEADER_SETS)
        self.srrip_leaders = [(i * step * 2) % NUM_SETS for i in range(NUM_LEADER_SETS)]
        self.brrip_leaders = [(i * step * 2 + step) % NUM_SETS for i in range(NUM_LEADER_SETS)]

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
            line_str = " | ".join(f"{line['tag']}({line['rrpv']})" for line in cache_set)
            print(f"Set {i:2d} {label}: {line_str}")
        follower_policy = "SRRIP" if self.psel >= self.psel_max // 2 else "BRRIP"
        print(f"PSEL={self.psel}, Follower policy={follower_policy}\n")
