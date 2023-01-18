#include <err.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "kvm_pt.h"
#include "shm_bitmap.h"

int vmx_pt_fd;
char *topa_buffer = NULL;

bool kvm_debug = false;

void enable_kvm_debug() {
    FILE *fp = fopen("/tmp/kvm_log", "w");
    fclose(fp);
    kvm_debug = true;
}

void kvm_debug_print(char * fmt, ...)
{
    if (kvm_debug == false)
        return;
    FILE *fp = fopen("/tmp/xdc_log", "a");
    va_list argptr;
    va_start(argptr,fmt);
    vfprintf(fp, fmt, argptr);
    va_end(argptr);
    fclose(fp);
}

int init_kafl_pt(int vcpufd) {
    int topa_size, ret;

    vmx_pt_fd = ioctl(vcpufd, KVM_VMX_PT_SETUP_FD, (unsigned long)0);
    if (vmx_pt_fd == -1) {
        perror("ioctl(KVM_VMX_PT_SETUP_FD): ");
        return -1;
    }
    kvm_debug_print("init_kafl_pt: vmx_pt_fd = %d\n", vmx_pt_fd);

    topa_size = ioctl(vmx_pt_fd, KVM_VMX_PT_GET_TOPA_SIZE, (unsigned long)0x0);
    if ( topa_size < 0 ) {
        perror("ioctl(KVM_VMX_PT_GET_TOPA_SIZE): ");
        return -1;
    }
    kvm_debug_print("init_kafl_pt: topa_size = %u\n", topa_size);

    topa_buffer = mmap(NULL, topa_size, PROT_READ, MAP_SHARED, vmx_pt_fd, 0);
    if ( topa_buffer == MAP_FAILED ){
        perror("mmap: ");
        return -1;
    }
    kvm_debug_print("init_kafl_pt: topa_buffer = 0x%lx\n", topa_buffer);

    ret = ioctl(vmx_pt_fd, KVM_VMX_PT_ENABLE, (unsigned long)0);
    if ( ret == -1 ){
        perror("ioctl(KVM_VMX_PT_ENABLE): ");
        return -1;
    }

    kvm_debug_print("init_kafl_pt: Enabled KVM-PT\n");
    return 0;
}

int clear_topa_buffer() {
    int topa_size;
    topa_size = ioctl(vmx_pt_fd, KVM_VMX_PT_CHECK_TOPA_OVERFLOW, (unsigned long)0);
    if ( topa_size > 0 ){
        kvm_debug_print("clear_topa_buffer: topa_size = %d\n", topa_size);
        return copy_topa_buffer(topa_buffer, topa_size);
    }
    return 0;
}