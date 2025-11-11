#! /usr/bin/python3

import random

# === Cache Configuration Macros ===
# NUM_SETS
# ASSOC
# MAX_RRPV
# INSERTION_POS

class SRRIPCache:
    def __init__(self, NUM_SETS, ASSOC, MAX_RRPV, INSERTION_POS, INIT_STATE):
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

        while True:
            for line in cache_set:
                if line['rrpv'] == self.max_rrpv:
                    line['tag'] = tag
                    line['rrpv'] = self.max_rrpv - 1
                    return False
            for line in cache_set:
                line['rrpv'] += 1

    def print_state(self):
        print_str = ''
        for i, cache_set in enumerate(self.sets):
            line_str = " | ".join(f"{line['tag']}({line['rrpv']})" for line in cache_set)
            print_str += f"Set {i:2d}: {line_str}\n"
        print_str += '\n'
        return print_str
