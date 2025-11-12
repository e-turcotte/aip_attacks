#include <x86intrin.h>
#include "inline_asm.h"
#include "pedit_wrap.h"
#include "slice_functions.h"
struct access_stream{
  int unique_entries;
  int sequence_length;
  int*  sequence;
  int** addresses;
};
uint8_t**  gen_evict_sets(uint64_t index, int slice);
uint64_t calibrate_latency();
uint64_t check_evict(uint8_t** evict_sets, uint64_t threshold, int size);
bool find_minimal_set_recursive(uint8_t** original_set, bool* is_active, 
                                int n, int current_size, int target_size, uint64_t threshold);
uint8_t** create_subset(uint8_t** original_set, bool* is_active, int n, int active_count);
int SIZE = 25;
int GOAL = 17;
int main(){
    if (ptedit_init()) {
      printf(TAG_FAIL "Error: Could not initalize PTEditor, did you load the kernel module?\n");
      return 1;
    }    
    uint64_t threshold = calibrate_latency();
    
    uint8_t** evict_sets = gen_evict_sets(68, 0);

    if(evict_sets[0] == 0){
        free(evict_sets);
        return 0;
    }

    uint64_t hit_cnt = check_evict(evict_sets, threshold, SIZE);
    
    if(hit_cnt == 0){
        bool* is_active = (bool*)malloc(sizeof(bool) * SIZE);
        for (int i = 0; i < SIZE; i++) {
            is_active[i] = true;
        }
        bool correct = find_minimal_set_recursive(evict_sets, is_active, SIZE, SIZE, GOAL, threshold);
        if(correct){
            printf("HURRAH! YOUR HASHING IS MESSED UP\n");
            uint8_t** smart_subset = create_subset(evict_sets,is_active, SIZE,GOAL);
            for(int i = 0; i < GOAL; i++){
                // printf("%llu Thresh | %llu Hit_Cnt/50\n", threshold, hit_cnt);
                if(evict_sets[i] != 0){
                    printf("0x%p, ",(uint64_t)virt2phys(evict_sets[i], false));
                }
            }   
            printf("\n");
            free(smart_subset);
        }
        free(is_active);
    }

    ptedit_cleanup();
    for(int i = 0; i < SIZE; i++){
        if(evict_sets[i] != 0){
            free(evict_sets[i]);
        }
    }
    free(evict_sets);
    return 0;
}
uint8_t** create_subset(uint8_t** original_set, bool* is_active, int n, int active_count) {
    uint8_t** subset = (uint8_t**)malloc(sizeof(uint8_t*) * active_count);
    if (!subset) {
        fprintf(stderr, "Failed to allocate memory for subset\n");
        return NULL;
    }

    int j = 0;
    for (int i = 0; i < n; i++) {
        if (is_active[i]) {
            subset[j] = original_set[i];
            j++;
        }
    }
    return subset;
}

bool find_minimal_set_recursive(uint8_t** original_set, bool* is_active, 
                                int n, int current_size, int target_size, uint64_t threshold){
    if (current_size == target_size) {
        // printf("Success! Found a working set of size %d.\n", target_size);
        return true; // We found a solution
    }
    for (int i = 0; i < n; i++) {
        
        // Skip items that are already removed
        if (!is_active[i]) {
            continue;
        }

        // "Remove" item i
        is_active[i] = false;
        int next_size = current_size - 1;

        // Create the new, smaller set to test
        uint8_t** test_set = create_subset(original_set, is_active, n, next_size);
        if (!test_set) return false; // Malloc failed
        
        // printf("Trying to remove item at original index %d...\n", i);
        
        // 3. Check if this smaller set still works
        if (check_evict(test_set, threshold, next_size)) {
            // It works! Recursively try to prune this *new* set
            bool found_solution = find_minimal_set_recursive(original_set, is_active, 
                                                             n, next_size, target_size, threshold);
            free(test_set); // Done with this test set

            if (found_solution) {
                return true; // Success! Propagate up.
            }

        } else {
            // It failed. This item is essential.
            free(test_set); // Done with this test set
        }

        // 4. Backtrack: "Add" item i back, since removing it failed
        //    (either 'check' failed, or the recursive path failed)
        // printf("Backtracking: Item %d is essential.\n", i);
        is_active[i] = true;
    }

    // 5. If we tried removing every item and no path led to a solution
    return false;
}

uint64_t check_evict(uint8_t** evict_sets, uint64_t threshold, int size){
    uint64_t hit_cnt = 0;
    for(int j = 0; j < 50; j++){
        for(int i = 0; i < size; i ++){
            _mm_clflush(evict_sets[i]);
        }
        _mm_mfence();
        uint64_t access_time = 0;
        for(int i = 0; i < size+1; i++){
            access_time = _time_maccess(evict_sets[i%size]);
            if(i == size && access_time < threshold){
                // printf("AT == %llu\n", access_time);
                hit_cnt += 1;
            }
            else if(i == size){
                // printf("AT == %llu\n", access_time);
            }
            _mm_mfence();
        }
    }
    if(hit_cnt == 0){
        // printf("Begin\n");
        // for(int i = 0; i < size; i++){
        //     // printf("%llu Thresh | %llu Hit_Cnt/50\n", threshold, hit_cnt);
        //     if(evict_sets[i] != 0){
        //         printf("0x%p, ",(uint64_t)virt2phys(evict_sets[i], false));
        //     }
        // }   
        // printf("\n");
    }
    return hit_cnt;
}


uint8_t** gen_evict_sets(uint64_t index, int slice){
    uint8_t** evict_sets = calloc( SIZE,sizeof(uint8_t*));
    int ctr = 0;
    while(ctr != SIZE){
        uint8_t* tmp = malloc(4096);        
        uint64_t phys = (uint64_t)virt2phys(tmp, false);
        int tmp_slice = compute_slice(phys,12);
        uint64_t tmp_idx = (phys >> 6) & 0x03FF;
        // printf("\n %llu \n", phys);
        // printf("\n %llu Slice | Index %llu \n", tmp_slice, tmp_idx);
        if(tmp_slice == slice &&  tmp_idx == index){
            // printf("\n Hit %llu \n", phys);
            evict_sets[ctr] = tmp;
            ctr += 1;
        }
        else{
            free(tmp);
        }
    }
    return evict_sets;
}

uint64_t calibrate_latency() {
    uint64_t hit = 0, miss = 0, threshold, rep = 1000;
    uint8_t *data = malloc(8);
    assert(data); // Lazy "exception" handling

    // Measure cache hit latency
    _maccess(data);
    for (uint32_t n = 0; n < rep; n++) {
        uint64_t start = _timer_start();
        _maccess(data);
        hit += _timer_end() - start;
    }
    hit /= rep;

    // Measure cache miss latency
    for (uint32_t n = 0; n < rep; n++) {
        _mm_clflush(data);
        uint64_t start = _timer_start();
        _maccess(data);
        miss += _timer_end() - start;
    }
    miss /= rep;

    threshold = ((2 * miss) + hit) / 3;
    // printf("Avg. hit latency: %" PRIu64 ", Avg. miss latency: %" PRIu64
    //        ", Threshold: %" PRIu64 "\n",
    //        hit, miss, threshold);
    free(data);
    return threshold;
}

