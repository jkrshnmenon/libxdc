#include "shm_bitmap.h"

void *page_cache_fetch(void *ptr, uint64_t page, bool *success) {
    // Hopefully this doesn't break anything
    *success = true;
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

    uint64_t filter[4][2] = {0};
    decoder = libxdc_init(filter, &page_cache_fetch, NULL, trace_map, MAP_SIZE);

    debug_print("Mapped shared bitmap @ 0x%lx\n", trace_map);
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
    memcpy(src, dst, size);

    // Append the bytes that libxdc wants to see
    ((uint8_t*)dst)[size] = PT_TRACE_END;

    // Hopefully this doesn't crash libxdc
    // But I don't care about filters
    decoder_result_t ret = libxdc_decode(decoder, dst, size);
    switch (ret) {
        case decoder_success:
            debug_print("decoder_success\n");
            return 0;
        case decoder_success_pt_overflow:
            debug_print("decoder_success_pt_overflow\n");
            return 0;
        case decoder_page_fault:        
            debug_print("decoder_page_fault\n");
            return 1;
        case decoder_error:             
            debug_print("decoder_error\n");
            return 2;
        case decoder_unkown_packet:     
            debug_print("decoder_unkown_packet\n");
            return 3;
    }

    return 0;
}