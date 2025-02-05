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
    FILE *fp = fopen("/tmp/kvm_log", "a");
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
    return 0;
}

int enable_kafl_pt() {
    int ret = ioctl(vmx_pt_fd, KVM_VMX_PT_ENABLE, (unsigned long)0);
    if ( ret == -1 ) {
        perror("ioctl(KVM_VMX_PT_ENABLE): ");
        return -1;
    }

    kvm_debug_print("init_kafl_pt: Enabled KVM-PT\n");
    return 0;
}


int add_ip_filter(uint64_t start, uint64_t end) {
    struct vmx_pt_filter_iprs filter_iprs;
    filter_iprs.a = start;
    filter_iprs.b = end;

    /* Set up ADDR0 IP filtering */
    int ret = ioctl(vmx_pt_fd, KVM_VMX_PT_CONFIGURE_ADDR0, &filter_iprs);
    if (ret == -1) {
         perror("ioctl(KVM_VMX_PT_CONFIGURE_ADDR0): ");
         return -1;
    }

    /* Enable ADDR0 IP filtering */
    ret = ioctl(vmx_pt_fd, KVM_VMX_PT_ENABLE_ADDR0, (unsigned long)0);
    if ( ret == -1 ) {
         perror("ioctl(KVM_VMX_PT_ENABLE_ADDR0): ");
         return -1;
    }
    return 0;
}


int add_cr3_filter(uint64_t cr3) {
    struct vmx_pt_multi_cr3 filter_cr3s;
    filter_cr3s.cr3_0 = cr3;
    int ret = ioctl(vmx_pt_fd, KVM_VMX_PT_CONFIGURE_MULTI_CR3, &filter_cr3s);
    if ( ret == -1 ) {
	    perror("ioctl(KVM_VMX_PT_CONFIGURE_MULTI_CR3): ");
	    return -1;
    }

    ret = ioctl(vmx_pt_fd, KVM_VMX_PT_ENABLE_MULTI_CR3, (unsigned long)0);
    if ( ret == -1 ) {
	    perror("ioctl(KVM_VMX_PT_ENABLE_MULTI_CR3): ");
	    return -1;
    }

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
