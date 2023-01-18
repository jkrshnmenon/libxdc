/* +----------------------------------------+
 * | KVM-PT configuration ioctls 	     	|
 * +----------------------------------------+ */
#define KVM_VMX_PT_SETUP_FD					_IO(KVMIO,	0xd0)			/* acquire KVM-PT fd (via vcpu fd ioctl)*/
#define KVM_VMX_PT_CONFIGURE_ADDR0			_IOW(KVMIO,	0xd1, __u64)	/* configure IP-filtering for addr0_a & addr0_b */
#define KVM_VMX_PT_CONFIGURE_ADDR1			_IOW(KVMIO,	0xd2, __u64)	/* configure IP-filtering for addr1_a & addr1_b */
#define KVM_VMX_PT_CONFIGURE_ADDR2			_IOW(KVMIO,	0xd3, __u64)	/* configure IP-filtering for addr2_a & addr2_b */
#define KVM_VMX_PT_CONFIGURE_ADDR3			_IOW(KVMIO,	0xd4, __u64)	/* configure IP-filtering for addr3_a & addr3_b */

#define KVM_VMX_PT_CONFIGURE_CR3			_IOW(KVMIO,	0xd5, __u64)	/* setup CR3 filter value */
#define KVM_VMX_PT_ENABLE					_IO(KVMIO,	0xd6)			/* enable and lock configuration */ 
#define KVM_VMX_PT_GET_TOPA_SIZE			_IOR(KVMIO,	0xd7, __u32)	/* get pre-defined ToPA size */
#define KVM_VMX_PT_DISABLE					_IO(KVMIO,	0xd8)			/* disable and release configuration */ 
#define KVM_VMX_PT_CHECK_TOPA_OVERFLOW		_IO(KVMIO,	0xd9)			/* get current pt data size */

#define KVM_VMX_PT_ENABLE_ADDR0				_IO(KVMIO,	0xaa)			/* enable IP-filtering for addr0 */
#define KVM_VMX_PT_ENABLE_ADDR1				_IO(KVMIO,	0xab)			/* enable IP-filtering for addr1 */
#define KVM_VMX_PT_ENABLE_ADDR2				_IO(KVMIO,	0xac)			/* enable IP-filtering for addr2 */
#define KVM_VMX_PT_ENABLE_ADDR3				_IO(KVMIO,	0xad)			/* enable IP-filtering for addr3 */

#define KVM_VMX_PT_DISABLE_ADDR0			_IO(KVMIO,	0xae)			/* disable IP-filtering for addr0 */
#define KVM_VMX_PT_DISABLE_ADDR1			_IO(KVMIO,	0xaf)			/* disable IP-filtering for addr1 */
#define KVM_VMX_PT_DISABLE_ADDR2			_IO(KVMIO,	0xe0)			/* disable IP-filtering for addr2 */
#define KVM_VMX_PT_DISABLE_ADDR3			_IO(KVMIO,	0xe1)			/* disable IP-filtering for addr3 */

#define KVM_VMX_PT_ENABLE_CR3				_IO(KVMIO,	0xe2)			/* enable CR3 filtering */
#define KVM_VMX_PT_DISABLE_CR3				_IO(KVMIO,	0xe3)			/* disable CR3 filtering */

#define KVM_VMX_PT_SUPPORTED				_IO(KVMIO,	0xe4)

#define KVM_VMX_PT_GET_ADDRN				_IO(KVMIO,	0xe9)

#define KVM_CAP_NYX_PT 512
#define KVM_CAP_NYX_FDL 513

/* +----------------------------------------+
 * | KVM-PT specific exit reasons 	    	|
 * +----------------------------------------+ */

/* New KVM exit reason to declare a full ToPA buffer */
#define KVM_EXIT_KAFL_TOPA_MAIN_FULL	119	



/* +----------------------------------------+
 * | KVM-PT specific structs and constants 	|
 * +----------------------------------------+ */
struct vmx_pt_filter_iprs {
	__u64 a;
	__u64 b;
};

/*
#define PAGE_SHIFT						12
#define TOPA_MAIN_ORDER					7
#define TOPA_FALLBACK_ORDER				0
#define TOPA_MAIN_SIZE					((1 << TOPA_MAIN_ORDER)*(1 << PAGE_SHIFT))
#define TOPA_FALLBACK_SIZE				((1 << TOPA_FALLBACK_ORDER)*(1 << PAGE_SHIFT))
#define TOPA_SIZE 						(TOPA_MAIN_SIZE + TOPA_FALLBACK_SIZE)
*/

//Debug support 
void enable_kvm_debug();
void kvm_debug_print(char *fmt, ...);

// Initialize KVM-PT
int init_kafl_pt(int);

// Should be called when the KVM_EXIT_KAFL_TOPA_MAIN_FULL is hit
int clear_topa_buffer();
