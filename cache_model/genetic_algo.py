#!python3

import random
import copy
import os
import multiprocessing as mp

from cache import run_cache

# ====================================
# Configurable parameters
# ====================================
POP_SIZE = 10
GENERATIONS = 10
MUTATION_RATE = 0.3
CROSSOVER_RATE = 0.5
LENGTH_PENALTY = 1   # Penalty per trace line
ALPHABET = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'L', 'M', 'N', 'O', 'P']
NUM_WORKERS = 8          # Adjust based on your CPU cores
OUTPUT_DIR = "traces/ga"  # Folder to save traces

# ====================================
# User-implemented stub
# ====================================
def evaluate_trace(trace_lines):
    srrip_avg_misses = 0
    brrip_avg_misses = 0
    with open(f"test_pop{trace_lines[0]}_srrip.trace", "w") as f:
        f.write("f traces/cyclic/sat_srrip.trace\n\n")
        f.write("\n".join(trace_lines[1]))
    with open(f"test_pop{trace_lines[0]}_brrip.trace", "w") as f:
        f.write("f traces/cyclic/sat_brrip.trace\n\n")
        f.write("\n".join(trace_lines[1]))
   
    for _ in range(10):
        run_cache("L1", f"_pop{trace_lines[0]}_srrip", f"test_pop{trace_lines[0]}_srrip.trace")
        run_cache("L2", f"_pop{trace_lines[0]}_srrip", f"L1_pop{trace_lines[0]}_srrip_miss.trace")
        run_cache("LLC", f"_pop{trace_lines[0]}_srrip", f"L2_pop{trace_lines[0]}_srrip_miss.trace")
        with open(f"LLC_pop{trace_lines[0]}_srrip_miss.trace", "r") as f:
            srrip_avg_misses += sum((0 if line.split(' ')[0] == 0 else 1) for line in f)
        
        run_cache("L1", f"_pop{trace_lines[0]}_brrip", f"test_pop{trace_lines[0]}_brrip.trace")
        run_cache("L2", f"_pop{trace_lines[0]}_brrip", f"L1_pop{trace_lines[0]}_brrip_miss.trace")
        run_cache("LLC", f"_pop{trace_lines[0]}_brrip", f"L2_pop{trace_lines[0]}_brrip_miss.trace")
        with open(f"LLC_pop{trace_lines[0]}_brrip_miss.trace", "r") as f:
            brrip_avg_misses += sum((0 if line.split(' ')[0] == 4 else 1) for line in f)
    
    srrip_avg_misses /= 10
    brrip_avg_misses /= 10

    os.remove(f"L1_pop{trace_lines[0]}_srrip_miss.trace")
    os.remove(f"L1_pop{trace_lines[0]}_srrip.log")
    os.remove(f"L2_pop{trace_lines[0]}_srrip_miss.trace")
    os.remove(f"L2_pop{trace_lines[0]}_srrip.log")
    os.remove(f"LLC_pop{trace_lines[0]}_srrip_miss.trace")
    os.remove(f"LLC_pop{trace_lines[0]}_srrip.log")
    os.remove(f"test_pop{trace_lines[0]}_srrip.trace")
    os.remove(f"L1_pop{trace_lines[0]}_brrip_miss.trace")
    os.remove(f"L1_pop{trace_lines[0]}_brrip.log")
    os.remove(f"L2_pop{trace_lines[0]}_brrip_miss.trace")
    os.remove(f"L2_pop{trace_lines[0]}_brrip.log")
    os.remove(f"LLC_pop{trace_lines[0]}_brrip_miss.trace")
    os.remove(f"LLC_pop{trace_lines[0]}_brrip.log")
    os.remove(f"test_pop{trace_lines[0]}_brrip.trace")
    
    return srrip_avg_misses, brrip_avg_misses



# ====================================
# Trace generation and mutation helpers
# ====================================
def random_pattern():
    length = 1 #random.randint(1, MAX_STRING_LEN)
    return ''.join(random.choices(ALPHABET, k=length))

def random_trace(num_lines=5):
    return [f"1 {random_pattern()}" for _ in range(num_lines)]

def mutate_trace(trace):
    new_trace = copy.deepcopy(trace)
    if random.random() < 0.3 and len(new_trace) > 1:
        # Remove a random line
        del new_trace[random.randrange(len(new_trace))]
    elif random.random() < 0.3:
        # Add a new random line
        new_trace.append(f"1 {random_pattern()}")
    else:
        # Modify an existing pattern
        idx = random.randrange(len(new_trace))
        new_trace[idx] = f"1 {random_pattern()}"
    return new_trace

def crossover_trace(trace1, trace2):
    if not trace1 or not trace2:
        return trace1
    cut1 = random.randint(0, len(trace1) - 1)
    cut2 = random.randint(0, len(trace2) - 1)
    new_trace = trace1[:cut1] + trace2[cut2:]
    # Renumber lines
    return [f"1 {line.split(maxsplit=1)[1]}" for line in new_trace]

# ====================================
# Fitness evaluation (parallelized)
# ====================================
def fitness(trace):
    try:
        A, B = evaluate_trace(trace)
        return abs(A - B) - LENGTH_PENALTY * len(trace) * len(trace)
    except Exception:
        return -1e9  # Invalid trace penalty

def evaluate_population(population):
    with mp.Pool(NUM_WORKERS) as pool:
        results = pool.map(fitness, list(enumerate(population)))
    return results

# ====================================
# Utility: save best trace to disk
# ====================================
def save_trace(trace_lines, generation, fitness_score):
    filename = OUTPUT_DIR + f"/best_gen{generation:02d}"
    with open(filename+".trace", "w") as f:
        f.write("\n".join(trace_lines))
    with open(filename+".meta", "w") as f:
        f.write(f"Fitness: {fitness_score:.4f}\n")
        f.write(f"Length: {len(trace_lines)}\n")
    return filename+".trace"

# ====================================
# Genetic Algorithm
# ====================================
def genetic_algorithm():
    population = [random_trace(50) for _ in range(POP_SIZE)]

    for gen in range(GENERATIONS):
        scores = evaluate_population(population)
        scored = sorted(zip(scores, population), key=lambda x: x[0], reverse=True)

        best_score, best_trace = scored[0]
        print(f"Gen {gen:02d}: Best fitness = {best_score:.4f}, length = {len(best_trace)}")

        # Save best trace of this generation
        path = save_trace(best_trace, gen, best_score)
        print(f"  → Saved best trace to {path}")

        # Selection (top 20%)
        survivors = [ind for _, ind in scored[:max(2, POP_SIZE // 5)]]

        # Generate next generation
        next_pop = survivors.copy()
        while len(next_pop) < POP_SIZE:
            if random.random() < CROSSOVER_RATE:
                p1, p2 = random.sample(survivors, 2)
                child = crossover_trace(p1, p2)
            else:
                child = copy.deepcopy(random.choice(survivors))
            if random.random() < MUTATION_RATE:
                child = mutate_trace(child)
            next_pop.append(child)

        population = next_pop

    # Final evaluation to get best individual
    final_scores = evaluate_population(population)
    best_score, best_trace = max(zip(final_scores, population), key=lambda x: x[0])

    # Save final best
    final_path = save_trace(best_trace, GENERATIONS, best_score)
    print(f"\nFinal best trace saved to {final_path}")
    return best_trace, best_score


# ====================================
# Main entry
# ====================================
if __name__ == "__main__":
    best_trace, score = genetic_algorithm()
    print("\nBest trace found:")
    print("\n".join(best_trace))
    print(f"Final fitness: {score:.4f}")

