#! /usr/bin/python3

import sys
import random

# === Cache Configuration Macros ===
NUM_SETS = 16
ASSOC = 8
INIT_STATE = True

class TreePLRUCache:
    def __init__(self):
        self.num_sets = NUM_SETS
        self.assoc = ASSOC
        self.max_tplru = ASSOC-1
        if INIT_STATE:
            self.trees = [[random.randint(0,1) for _ in range(self.max_tplru)] for _ in range(NUM_SETS)]
            self.sets = [[{'tag': '~init', 'tplru': 0} for _ in range(ASSOC)] for _ in range(NUM_SETS)]
        else:
            self.trees = [[0 for _ in range(self.max_tplru)] for _ in range(NUM_SETS)]
            self.sets = [[{'tag': '~empty', 'tplru': 0} for _ in range(ASSOC)] for _ in range(NUM_SETS)]
        self.tree_lvls = 0
        while 2**self.tree_lvls < ASSOC:
            self.tree_lvls = self.tree_lvls + 1
        for idx in range(NUM_SETS):
            self._eval_tree(self.sets[idx], self.trees[idx])

    def access(self, set_idx, tag):
        cache_set = self.sets[set_idx]
        tree = self.trees[set_idx]

        for way_idx, line in enumerate(cache_set):
            if line['tag'] == tag:
                self._set_tree(cache_set, tree, way_idx)
                self._eval_tree(cache_set, tree)
                return True

        with open('tplru_miss.trace', 'a') as f:
            f.write(f"{set_idx} {tag}\n")

        while True:
            for way_idx, line in enumerate(cache_set):
                if line['tplru'] == self.max_tplru:
                    line['tag'] = tag
                    self._set_tree(cache_set, tree, way_idx)
                    self._eval_tree(cache_set, tree)
                    return False

    def _set_tree(self, cache_set, tree, way_idx):
        lvl = 0
        mid_way = ASSOC / 2
        tree_idx = 0
        while lvl < self.tree_lvls:
            tree[tree_idx] = 1 if way_idx < mid_way else 0
            lvl = lvl + 1
            tree_idx = 1 + (0 if way_idx < mid_way else 2**(self.tree_lvls-lvl)-1)
            mid_way = mid_way + ASSOC/2**(lvl+1) * (-1 if way_idx < mid_way else 1)

        for line in cache_set:
            line['tplru'] = 0


    def _eval_tree(self, subset, limb):
        for i, way in enumerate(subset):
            way['tplru'] = way['tplru']*2 + (limb[0] if i >= len(subset)/2 else limb[0]^1)

        if len(subset) == 2 and len(limb) == 1:
            return

        self._eval_tree(subset[:int(len(subset)/2)], limb[1:int((len(limb)-1)/2)+1])
        self._eval_tree(subset[int(len(subset)/2):], limb[int((len(limb)-1)/2)+1:])


    def print_state(self):
        for i, cache_set in enumerate(self.sets):
            line_str = " | ".join(f"{line['tag']}({line['tplru']})" for line in cache_set)
            print(f"Set {i:2d} TREE[{''.join(str(n) for n in self.trees[i])}]: {line_str}")
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
    cache = TreePLRUCache()

    if len(sys.argv) == 2:
        run_trace(sys.argv[1], cache)
        print(f"\n===Exiting cache trace, Entering interactive mode===\n\n")
    #cache.print_state()
    #run_interactive(cache)

