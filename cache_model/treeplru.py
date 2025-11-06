#! /usr/bin/python3

import random

# === Cache Configuration Macros ===
# NUM_SETS
# ASSOC

class TreePLRUCache:
    def __init__(self, NUM_SETS, ASSOC, INIT_STATE):
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
                return True

        while True:
            for way_idx, line in enumerate(cache_set):
                if line['tplru'] == self.max_tplru:
                    line['tag'] = tag
                    self._set_tree(cache_set, tree, way_idx)
                    return False

    def _set_tree(self, cache_set, tree, way_idx):
        lvl = 0
        mid_way = self.assoc / 2
        tree_idx = 0
        while lvl < self.tree_lvls:
            tree[tree_idx] = 1 if way_idx < mid_way else 0
            lvl = lvl + 1
            tree_idx = tree_idx + 1 + (0 if way_idx < mid_way else 2**(self.tree_lvls-lvl)-1)
            mid_way = mid_way + self.assoc/2**(lvl+1) * (-1 if way_idx < mid_way else 1)

        for line in cache_set:
            line['tplru'] = 0

        self._eval_tree(cache_set, tree)


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
