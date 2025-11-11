#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "../PTEditor/ptedit_header.h"

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

int main(){
    if (ptedit_init()) {
      printf(TAG_FAIL "Error: Could not initalize PTEditor, did you load the kernel module?\n");
      return 1;
    }    
    int* test = malloc(sizeof(int));
    printf("%p == test\n", test);
   
    size_t res = virt2phys(test, true);
    printf("%zx == res\n", res);
    ptedit_cleanup();
    printf(TAG_OK "Done\n");
    free(test);
    return 0;
}

size_t virt2phys(void* ptr, bool print){
    size_t phys = 0;
    ptedit_entry_t entry = ptedit_resolve(ptr, 0);
    if(is_normal_page(entry.pd)) {
        if(print) printf(TAG_PROGRESS "Page is 4KB\n");
        ptedit_print_entry(entry.pte);
        phys = (ptedit_get_pfn(entry.pte) << 12) | (((size_t)ptr) & 0xfff);
    } else {
        if(print) printf(TAG_PROGRESS "Page is 2MB\n");
        ptedit_print_entry(entry.pd);
        phys = (ptedit_get_pfn(entry.pd) << 12) | (((size_t)ptr) & 0x1fffff);
    }
    if(print){
        printf(TAG_OK "Virtual address: %p\n", ptr);
        printf(TAG_OK "Physical address: 0x%zx\n", phys);
    }
    return phys;
}