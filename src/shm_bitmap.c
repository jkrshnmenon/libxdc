#include "shm_bitmap.h"
#include "afl_hash.h"

bool xdc_debug = false;
char *trace_map = (char *)&dummy;

void enable_xdc_debug() {
    FILE *fp = fopen("/tmp/xdc_log", "w");
    fclose(fp);
    xdc_debug = true;
}

void xdc_debug_print(char * fmt, ...)
{
    if (xdc_debug == false)
        return;
    FILE *fp = fopen("/tmp/xdc_log", "a");
    va_list argptr;
    va_start(argptr,fmt);
    vfprintf(fp, fmt, argptr);
    va_end(argptr);
    fclose(fp);
}

void *page_cache_fetch(void *ptr, uint64_t page, bool *success) {
    // Hopefully this doesn't break anything
    *success = true;
}

static void update_bitmap(void *opaque, uint64_t src, uint64_t dst) {
    xdc_debug_print("update_bitmap: [src] 0x%lx -> [dst] 0x%lx\n", src, dst);

    uint64_t prev_loc = (uint64_t)(afl_hash_ip(src)) & (MAP_SIZE - 1);
    uint64_t cur_loc = (uint64_t)(afl_hash_ip(dst)) & (MAP_SIZE - 1);

    uint64_t afl_idx = cur_loc ^ (prev_loc >> 1);
    trace_map[afl_idx]++;
}

int create_shared_bitmap() {
    char *shm_env_var = getenv(SHM_ENV_VAR);
    if (shm_env_var == NULL) {
        fprintf(stderr, "SHM_ENV_VAR not set\n");
        return -1;
    }

    int shm_id = atoi(shm_env_var);
    trace_map = (char *)shmat(shm_id, NULL, 0);

    if ( trace_map == (char *)-1 ) {
        perror("shmat:");
        return -1;
    }
    return 0;
}

int init_decoder() {
    if ( trace_map == NULL ) {
        fprintf(stderr, "trace_map has not been initialized\n");
        return -1;
    }
    uint64_t filter[4][2] = {0};
    memset(trace_map, 0, MAP_SIZE);
    decoder = libxdc_init(filter, &page_cache_fetch, NULL, trace_map, MAP_SIZE);
    libxdc_enable_tracing(decoder);
    libxdc_register_edge_callback(decoder, &update_bitmap, (void *)NULL);
    // libxdc_register_bb_callback(decoder, update_bitmap, (void *)NULL);
    // afl_prev_loc = 0;
    xdc_debug_print("Mapped shared bitmap @ 0x%lx\n", trace_map);
    return 0;
}


int copy_topa_buffer(char *src, size_t size) {
    // Double check that the shared bitmap has been created
    if ( trace_map == NULL) {
        fprintf(stderr, "trace_map has not been initialized\n");
        return -1;
    }

    // Create a new buffer and include space for PT_TRACE_END
    void *dst = malloc(size + 16);
    if ( dst == NULL ){
        perror("malloc:");
        return -1;
    }

    // Copy the topa buffer into the new buffer
    xdc_debug_print("Copying 0x%lx -> 0x%lx\n", src, dst);
    memcpy(dst, src, size);
    xdc_debug_print("Done copying 0x%lx -> 0x%lx\n", src, dst);

    // Append the bytes that libxdc wants to see
    ((uint8_t*)dst)[size] = PT_TRACE_END;

    // Hopefully this doesn't crash libxdc
    // But I don't care about filters
    decoder_result_t ret = libxdc_decode(decoder, dst, size);

    free(dst);

    switch (ret) {
        case decoder_success:
            xdc_debug_print("decoder_success\n");
            return 0;
        case decoder_success_pt_overflow:
            xdc_debug_print("decoder_success_pt_overflow\n");
            return 0;
        case decoder_page_fault:        
            xdc_debug_print("decoder_page_fault\n");
            return 1;
        case decoder_error:             
            xdc_debug_print("decoder_error\n");
            return 2;
        case decoder_unkown_packet:     
            xdc_debug_print("decoder_unkown_packet\n");
            return 3;
    }

    return 0;
}
