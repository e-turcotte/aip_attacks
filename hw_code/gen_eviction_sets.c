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
int SIZE = 25;

int main(){
    if (ptedit_init()) {
      printf(TAG_FAIL "Error: Could not initalize PTEditor, did you load the kernel module?\n");
      return 1;
    }    
    uint64_t threshold = calibrate_latency();
    uint64_t hit_cnt = 0;
    uint8_t** evict_sets = gen_evict_sets(68, 0);

    if(evict_sets[0] == 0){
        free(evict_sets);
        return 0;
    }
    for(int j = 0; j < 50; j++){
        for(int i = 0; i < SIZE; i ++){
            _mm_clflush(evict_sets[i]);
        }
        _mm_mfence();
        uint64_t access_time = 0;
        for(int i = 0; i < SIZE+1; i++){
            access_time = _time_maccess(evict_sets[i%SIZE]);
            if(i == SIZE && access_time < threshold){
                // printf("AT == %llu\n", access_time);
                hit_cnt += 1;
            }
            else if(i == SIZE){
                // printf("AT == %llu\n", access_time);
            }
            _mm_mfence();
        }
    }
    if(hit_cnt == 0){
        for(int i = 0; i < SIZE; i++){
            printf("%llu Thresh | %llu Hit_Cnt/50\n", threshold, hit_cnt);
            if(evict_sets[i] != 0){
                printf("%llu, ",(uint64_t)virt2phys(evict_sets[i], false));
            }
        }   
        printf("\n");
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

