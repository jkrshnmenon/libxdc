#pragma once
#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>

#include "libxdc.h"
#include "decoder.h"

#define SHM_ENV_VAR         "XDC_SHM"
#define MAP_SIZE         0x10000
#define PT_TRACE_END			__extension__ 0b01010101

#ifdef BITMAP_DEBUG
#define debug_print(msg, ...) (fprintf(stderr, msg, ##__VA_ARGS__))
#else
#define debug_print(msg, ...) (void)0
#endif

char *trace_map;
libxdc_t* decoder;

void *page_cache_fetch(void *, uint64_t, bool *);

// This function will be invoked by afl_sync to create a shared memory page
// This shared memory page will be used to sync the bitmap.
int create_shared_bitmap();

// This function will be invoked by firecracker to copy the topa buffer
// It is also responsible for generating the bitmap from the PT packet buffer
int copy_topa_buffer(char *src, size_t size);

