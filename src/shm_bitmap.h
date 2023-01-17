#pragma once
#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>

#include "libxdc.h"
#include "decoder.h"

#define SHM_ENV_VAR         "__AFL_SHM_ID"
#define MAP_SIZE         0x10000
#define PT_TRACE_END			__extension__ 0b01010101

char dummy[MAP_SIZE];
char *trace_map = &dummy;
libxdc_t* decoder;
uint64_t afl_prev_loc;

// Debug support
void enable_debug();
void debug_print(char *fmt, ...);

// Just a dummy function to make libxdc happy
void *page_cache_fetch(void *, uint64_t, bool *);

// A function that's going to be invoked on each bb
void update_bitmap(void *, uint64_t, uint64_t);

// This function will be invoked by afl_sync to create a shared memory page
// This shared memory page will be used to sync the bitmap.
int create_shared_bitmap();

// This function will be invoked by firecracker to copy the topa buffer
// It is also responsible for generating the bitmap from the PT packet buffer
int copy_topa_buffer(char *src, size_t size);

