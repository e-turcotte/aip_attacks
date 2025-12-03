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
int send_bit(int srrip, uint8_t** leader_evset, uint8_t** follower_evset);
uint8_t** get_pruned_evic_set(uint8_t** free_sets, uint64_t index, int slice, bool* suc);
uint8_t**  gen_evict_sets(uint64_t index, int slice, uint8_t** free_sets);
uint64_t calibrate_latency();
uint64_t check_evict(uint8_t** evict_sets, uint64_t threshold, int size);
uint64_t check_evict_full(uint8_t** evict_sets, uint64_t threshold, int size);

bool find_minimal_set_recursive(uint8_t** original_set, bool* is_active, 
                                int n, int current_size, int target_size, uint64_t threshold);
uint8_t** create_subset(uint8_t** original_set, bool* is_active, int n, int active_count);

int SIZE = 128;
int GOAL = 17;

int idx_seq[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 8, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 15, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 5, 7, 15, 12, 3, 1, 10, 10, 9, 12, 11, 10, 14, 7, 4, 6, 5, 9, 6, 4, 4, 13, 1, 14, 1, 0, 6, 12, 14, 14, 3, 12, 6, 14, 14, 12, 12, 9, 15, 3, 3, 0, 15, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 5, 4, 6, 6, 5, 2, 15, 1, 2, 4, 5, 8, 10, 3, 7, 13, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 15, 8, 5, 4, 14, 13, 1, 15, 3, 10, 14, 1, 7, 8, 1, 4, 9, 13, 4, 10, 2, 6, 0, 6, 3, 8, 7, 8, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 6, 6, 5, 4, 2, 15, 1, 2, 4, 5, 8, 10, 3, 7, 13, 11, 7, 13, 5, 0, 3, 4, 2, 10, 14, 7, 4, 6, 5, 9, 6, 4, 4, 13, 1, 14, 1, 0, 6, 12, 14, 14, 15, 6, 14, 14, 7, 5, 5, 10, 3, 9, 4, 8, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 9, 10, 0, 11, 12, 1, 13, 5, 6, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 15, 8, 0, 15, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 5, 4, 15, 6, 6, 6, 6, 8, 7, 8, 9, 10, 0, 11, 12, 3, 4, 5, 6, 7, 8, 7, 8, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 15, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 5, 4, 15, 6, 6, 6, 6, 8, 7, 8, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 8, 15, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 11, 12, 1, 2, 4, 9, 15, 4, 9, 13, 0, 8, 0, 11, 8, 7, 2, 5, 12, 15, 7, 8, 13, 1, 15, 13, 3, 5, 3, 7, 9, 6, 8, 15, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 10, 0, 7, 8, 7, 8, 9, 10, 0, 11, 12, 8, 11, 12, 1, 2, 4, 9, 15, 4, 9, 13, 0, 8, 0, 11, 8, 7, 2, 5, 12, 15, 7, 8, 13, 1, 15, 13, 3, 5, 3, 7, 9, 6, 8, 15, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 10, 0, 7, 8, 7, 8, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 10, 3, 10, 3, 7, 13, 11, 7, 13, 5, 0, 3, 4, 2, 10, 14, 7, 4, 6, 5, 9, 6, 4, 4, 13, 1, 14, 1, 0, 6, 12, 14, 14, 15, 12, 6, 14, 14, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 15, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 5, 7, 15, 12, 3, 1, 10, 10, 9, 12, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 10, 0, 7, 8, 7, 8, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 10, 3, 10, 3, 7, 13, 11, 7, 13, 5, 0, 3, 4, 2, 10, 14, 7, 4, 6, 5, 9, 6, 4, 4, 13, 1, 14, 1, 0, 6, 12, 14, 14, 15, 12, 6, 14, 14, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 15, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 8, 5, 4, 14, 7, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 9, 10, 0, 11, 12, 1, 13, 5, 6, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 15, 8, 0, 15, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 5, 4, 15, 6, 6, 6, 6, 8, 7, 8, 9, 10, 0, 11, 12, 3, 4, 5, 6, 7, 8, 7, 8, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 15, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 5, 4, 15, 6, 6, 6, 6, 8, 7, 8, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 8, 15, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 11, 12, 1, 2, 4, 9, 15, 4, 9, 13, 0, 8, 0, 11, 8, 7, 2, 5, 12, 15, 7, 8, 13, 1, 15, 13, 3, 5, 3, 7, 9, 6, 8, 15, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 10, 0, 7, 8, 7, 8, 9, 10, 0, 11, 12, 1, 13, 5, 4, 7, 5, 5, 10, 3, 9, 4, 8, 14, 6, 1, 15, 7, 12, 3, 3, 0, 15, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 5, 7, 15, 12, 3, 1, 10, 10, 9, 12, 11, 10, 14, 7, 4, 6, 5, 9, 6, 4, 4, 13, 1, 14, 1, 6, 12, 14, 14, 3, 12, 6, 14, 14, 12, 12, 9, 15, 3, 3, 0, 15, 8, 5, 4, 14, 7, 12, 6, 2, 7, 13, 11, 9, 5, 4, 15, 6, 6, 6, 6, 8, 7, 14, 10, 5, 9};

int main(){
    if (ptedit_init()) {
      printf(TAG_FAIL "Error: Could not initalize PTEditor, did you load the kernel module?\n");
      return 1;
    }    
    
    bool* suc = malloc(sizeof(bool));
    if (suc == NULL) {
        printf("Error: Failed to allocate memory for 'suc'\n");
        return 1;
    }
    
	uint8_t** free_sets1;
    suc[0] = false;
	uint8_t** leader_evset;
    while(!suc[0]){
        free_sets1 = calloc(SIZE,sizeof(uint8_t*));
        leader_evset = get_pruned_evic_set(free_sets1, 0, 1, suc);
        // printf("%d \n", suc[0]);
        if(!suc[0]){
            for(int i = 0; i < SIZE; i++){
                if(free_sets1[i] != 0){
                    free(free_sets1[i]);
                }
            }
            free(free_sets1);
            free(leader_evset);
        }   
    }

	
	for(int j = 0; j < 50; j++){
		for(int j = 1; j < GOAL; j++){
        	_maccess(leader_evset[j]);
            _mm_mfence();
		}
	}
    

	uint8_t** free_sets2;
    suc[0] = false;
	uint8_t** follower_evset;
    while(!suc[0]){
        free_sets2 = calloc( SIZE,sizeof(uint8_t*));
        follower_evset = get_pruned_evic_set(free_sets2, 1, 1, suc);
        // printf("%d \n", suc[0]);
        if(!suc[0]){
            for(int i = 0; i < SIZE; i++){
                if(free_sets2[i] != 0){
                    free(free_sets2[i]);
                }
            }
            free(free_sets2);
            free(follower_evset);
        }   
    }
    
	int recv = 0;
	int msg = 67;
	int snd = msg;
	printf("=======CALIBRATING=======\n");
	send_bit(0, leader_evset, follower_evset);
	send_bit(1, leader_evset, follower_evset);

	int time_sum = 0;
	int num_bits = 0;
	int times[] = {0, 0, 0, 0, 0, 0, 0};

	uint64_t t_start = _timer_start();
	printf("\n\n=======SENDING=======\n");
	while(snd > 0){
		int t = send_bit(snd & 0x1, leader_evset, follower_evset);
		time_sum += t;
		times[6-num_bits] = t;
		num_bits++;

		snd = snd >> 1;
	}
	int time_avg = time_sum / num_bits;
	printf("\n\n=======RECEIVING=======\n");
	printf("BIT CUTOFF: %d\n", time_avg);
	for(int i = 0; i < num_bits; i++){
		printf("BIT[%d]: %d --> %d\n", i, times[i], (times[i] > time_avg)? 0 : 1);

		recv = (recv << 1) | ((times[i] > time_avg)? 0 : 1);	
	}
	uint64_t t_end = _timer_end();

	printf("\n\n=======RESULTS=======\n");
	printf("SENT: %d, RECEIVED: %d\n", msg, recv);
	printf("Time for message: %d\n", t_end-t_start);

    ptedit_cleanup();
    for(int i = 0; i < SIZE; i++){
        if(free_sets1[i] != 0){
            free(free_sets1[i]);
        }
        if(free_sets2[i] != 0){
            free(free_sets2[i]);
        }
    }
    free(free_sets1);
    free(free_sets2);
    free(leader_evset);
    free(follower_evset);
    free(suc);
    return 0;
}

int send_bit(int srrip, uint8_t** leader_evset, uint8_t** follower_evset){
    uint64_t access_time = 0;
	uint64_t threshold = calibrate_latency();
	uint64_t tally = 0;
    for(int k = 0; k < 100; k++){
		for(int i = 0; i < 50; i++){
			for(int j = srrip; j < GOAL; j++){
        		_maccess(leader_evset[j]);
            	_mm_mfence();
			}
		}
		for(int i = 0; i < GOAL; i++){
			_mm_clflush(follower_evset[i]);
			_mm_mfence();
		}
		uint64_t t_start = _timer_start();
		for(int i = 0; i < 1176; i++){
        	_maccess(follower_evset[idx_seq[i]]);
            _mm_mfence();
		}
		uint64_t t_end = _timer_end();
		tally += t_end - t_start;
	}

	printf("%d: %d\n", srrip, tally/100);
	return tally/100;
	
}

uint8_t**  get_pruned_evic_set(uint8_t** free_sets, uint64_t index, int slice, bool* suc ){
    uint64_t threshold = calibrate_latency();
    uint8_t** evict_sets = gen_evict_sets(index, slice, free_sets);
    
    if(free_sets[0] == 0){
        free(free_sets);
        return 0;
    }

    uint64_t hit_cnt = check_evict(evict_sets, threshold, SIZE);
    
    if(hit_cnt == 0){
        // printf("HITCNT == 0\n");
        bool* is_active = (bool*)malloc(sizeof(bool) * SIZE);
        for (int i = 0; i < SIZE; i++) {
            is_active[i] = true;
        }
        bool correct = find_minimal_set_recursive(evict_sets, is_active, SIZE, SIZE, GOAL, threshold);
        if(correct){
            uint8_t** smart_subset = create_subset(evict_sets,is_active, SIZE,GOAL);
            if(check_evict(evict_sets, threshold, SIZE) == 0 && check_evict(smart_subset, threshold, GOAL) == 0){
                    //printf("HURRAH! YOUR HASHING IS MESSED UP\n");
                    for(int i = 0; i < GOAL; i++){
                    // printf("%llu Thresh | %llu Hit_Cnt/50\n", threshold, hit_cnt);
                    if(evict_sets[i] != 0){
                        // printf("l:0x%llu, ",(uint64_t)virt2phys(smart_subset[i], false));
                        printf("0x%zx, ",(uint64_t)virt2phys(smart_subset[i], false));
                    }
                }   
                printf("\n");
                printf("Base Miss Vector: ");
                check_evict_full(evict_sets, threshold, SIZE);
                printf("Pruned Miss Vector: ");
                check_evict_full(smart_subset, threshold, GOAL);
                printf("\n");
                free(evict_sets);
                free(is_active);
                suc[0] = true;
                return smart_subset;
            }
            
            free(smart_subset);
        }
        free(is_active);
    }
    free(evict_sets);
    suc[0] = false;
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
        if (check_evict(test_set, threshold, next_size) == 0) {
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
    uint64_t access_time = 0;

    _mm_clflush(evict_sets[0]);
    _mm_mfence();
    uint8_t* victim = evict_sets[0];
    access_time = (volatile) _time_maccess(victim);
    _mm_mfence();

    for(int i = 1; i < size; i++){
    	victim = evict_sets[i%size];
        access_time = (volatile) _time_maccess(victim);
        _mm_mfence();
    }
    
    victim = evict_sets[0];
    access_time = (volatile) _time_maccess(victim);
    _mm_mfence();

    for(int j = 0; j < 4; j++){
        for(int i = 1; i < size; i++){
            uint8_t* a = evict_sets[i%size];
            access_time = (volatile) _time_maccess(a);
            _mm_mfence();
        }
    }
    access_time = (volatile) _time_maccess(victim);
    if(access_time < threshold){
        hit_cnt += 1;
    }
    _mm_mfence();
    return hit_cnt;
}

uint64_t check_evict_full(uint8_t** evict_sets, uint64_t threshold, int size){
    uint64_t hit_cnt = 0;
    uint8_t* victim = evict_sets[0];
    uint8_t* tmp = evict_sets[0];
    uint64_t miss_vect = 0;
    for(int i = 1; i <= size; i++){
        uint64_t tmp_hit_cnt = check_evict(evict_sets, threshold, size);
        if(tmp_hit_cnt){
            hit_cnt += tmp_hit_cnt;
            miss_vect = (miss_vect << 1 ) | 1;
        }
        else{
            miss_vect = (miss_vect << 1 ) ;
        }
        if(i != size){
            tmp = evict_sets[0];
            evict_sets[0] = evict_sets[i];
            evict_sets[i] = tmp;
        }
        
    }
    printf("0x%llx\n", miss_vect);
    return hit_cnt;
}

uint8_t** gen_evict_sets(uint64_t index, int slice, uint8_t** free_sets){
    uint8_t** evict_sets = calloc( SIZE,sizeof(uint8_t*));
    int ctr = 0;
    while(ctr != SIZE){
        uint8_t* page = malloc(4096);   
        
        bool found = false;
        for(int i = 0; i < 2048; i+=64){
            uint8_t* tmp = page + i;
            uint64_t phys = (uint64_t)virt2phys(tmp, false);
            int tmp_slice = compute_slice(phys,12);
            // int tmp_slice = get_address_slice(phys);
            uint64_t tmp_idx = (phys >> 6) & 0x03FF;
            // printf("\n %llu \n", phys);
            // printf("\n %llu Slice | Index %llu \n", tmp_slice, tmp_idx);
            if(tmp_slice == slice &&  tmp_idx == index){
                // printf("\n Hit %llu \n", phys);
                evict_sets[ctr] = tmp;
                free_sets[ctr] = page;
                ctr += 1;
                found = true;
                break;
            }
        }       
        if(!found){
            free(page);
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

