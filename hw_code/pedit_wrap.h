#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#define INTEL_GEN8_6_SLICE
#include "util.h"
#include "../PTEditor/ptedit_header.h"
// #include "../Slice-Slice-Baby/src/slice_partitioning/slicing.c"



#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RESET "\x1b[0m"

#define TAG_OK COLOR_GREEN "[+]" COLOR_RESET " "
#define TAG_FAIL COLOR_RED "[-]" COLOR_RESET " "
#define TAG_PROGRESS COLOR_YELLOW "[~]" COLOR_RESET " "
size_t virt2phys(void* ptr, bool print);

unsigned long target;






int is_normal_page(size_t entry) {
#if defined(__i386__) || defined(__x86_64__)
  return !(entry & (1ull << PTEDIT_PAGE_BIT_PSE));
#elif defined(__aarch64__)
  return 1;
#endif
}

// int main(){
//     if (ptedit_init()) {
//       printf(TAG_FAIL "Error: Could not initalize PTEditor, did you load the kernel module?\n");
//       return 1;
//     }    
//     int* test = malloc(sizeof(int));
//     printf("%p == test\n", test);
   
//     size_t res = virt2phys(test, true);
//     printf("%zx == res\n", res);
//     ptedit_cleanup();
//     printf(TAG_OK "Done\n");
//     free(test);
//     return 0;
// }


const int seq_len = 128;
const int reduction_bits = 7;
const int addr_bits = 39;
const uint64_t xor_mask[7] = {0x21ae7be000ULL, 0x435cf7c000ULL, 0x2717946000ULL, 0x4e2f28c000ULL, 0x1c5e518000ULL, 0x38bca30000ULL, 0x50d73de000ULL};
const int base_sequence[128] = {0, 1, 2, 3, 1, 4, 3, 4, 1, 0, 3, 2, 0, 5, 2, 5, 1, 0, 3, 2, 0, 5, 2, 5, 0, 5, 2, 5, 1, 4, 3, 4, 0, 1, 2, 3, 5, 0, 5, 2, 5, 0, 5, 2, 4, 1, 4, 3, 1, 0, 3, 2, 4, 1, 4, 3, 4, 1, 4, 3, 5, 0, 5, 2, 2, 3, 0, 1, 5, 2, 5, 0, 3, 2, 1, 0, 4, 3, 4, 1, 3, 2, 1, 0, 4, 3, 4, 1, 4, 3, 4, 1, 5, 2, 5, 0, 2, 3, 0, 1, 3, 4, 1, 4, 3, 4, 1, 4, 2, 5, 0, 5, 3, 2, 1, 0, 2, 5, 0, 5, 2, 5, 0, 5, 3, 4, 1, 4};
int get_seq_len()
{
    return seq_len;
}

int get_reduction_bits()
{
    return reduction_bits;
}

const int *get_base_sequence()
{
    return (int *)base_sequence;
}

const uint64_t *get_xor_mask()
{
    return (uint64_t *)xor_mask;
}

int get_address_slice(uint64_t address)
{
    if ((address >> (addr_bits + 1)) > 0)
    {
        return -1;
    }

    int slice = 0;
    for (int b = 0; b < reduction_bits; ++b)
    {
        slice |= ((__builtin_popcountll(xor_mask[b] & address) % 2) << b);
    }

    if (seq_len > 0)
    {
        int sequence_offset = ((uint64_t)address % (seq_len * CACHELINE)) >> 6;
        slice = base_sequence[sequence_offset ^ slice];
    }

    return slice;
}

size_t virt2phys(void* ptr, bool print){
    size_t phys = 0;
    ptedit_entry_t entry = ptedit_resolve(ptr, 0);
    if(is_normal_page(entry.pd)) {
        if(print) printf(TAG_PROGRESS "Page is 4KB\n");
        // ptedit_print_entry(entry.pte);
        phys = (ptedit_get_pfn(entry.pte) << 12) | (((size_t)ptr) & 0xfff);
    } else {
        if(print) printf(TAG_PROGRESS "Page is 2MB\n");
        // ptedit_print_entry(entry.pd);
        phys = (ptedit_get_pfn(entry.pd) << 12) | (((size_t)ptr) & 0x1fffff);
    }
    if(print){
        printf(TAG_OK "Virtual address: %p\n", ptr);
        printf(TAG_OK "Physical address: 0x%zx\n", phys);
    }
    uint64_t slice = get_address_slice((uint64_t)phys);
    if(print) printf("%llu == Slice\n", slice); 
    return phys;
}