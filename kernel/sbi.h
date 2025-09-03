#ifndef OS_KERNEL_SBI
#define OS_KERNEL_SBI

#include "types.h"

#define SBI_SPEC_VERSION_MAJOR(x) (((x) >> 6) & 0xFF)
#define SBI_SPEC_VERSION_MINOR(x) ((x) & 0xFFFFFF)

struct sbiret
{
    long error;
    long value;
};

enum sbierr
{
    SBI_SUCCESS               =  0,
    SBI_ERR_FAILED            = -1,
    SBI_ERR_NOT_SUPPORTED     = -2,
    SBI_ERR_INVALID_PARAM     = -3,
    SBI_ERR_DENIED            = -4,
    SBI_ERR_INVALID_ADDRESS   = -5,
    SBI_ERR_ALREADY_AVAILABLE = -6,
    SBI_ERR_ALREADY_STARTED   = -7,
    SBI_ERR_ALREADY_STOPPED   = -8,
    SBI_ERR_NO_SHMEM          = -9
};

enum sbiext
{
    SBI_EXTENSION_BASE = 0x10,
    SBI_EXTENSION_TIME = 0x54494D45,
    SBI_EXTENSION_IPI  = 0x735049,
    SBI_EXTENSION_RFNC = 0x52464E43,
    SBI_EXTENSION_HSM  = 0x48534D,
    SBI_EXTENSION_SRST = 0x53525354,
    SBI_EXTENSION_PMU  = 0x504D55,
    SBI_EXTENSION_DBCN = 0x4442434E,
    SBI_EXTENSION_SUSP = 0x53555350,
    SBI_EXTENSION_CPPC = 0x43505043,
    SBI_EXTENSION_NACL = 0x4E41434C,
    SBI_EXTENSION_STA  = 0x535441,
    SBI_EXTENSION_FWFT = 0x46574654,
};

enum sbistate
{
    SBI_STATE_STARTED         = 0,
    SBI_STATE_STOPPED         = 1,
    SBI_STATE_START_PENDING   = 2,
    SBI_STATE_STOP_PENDING    = 3,
    SBI_STATE_SUSPENDED       = 4,
    SBI_STATE_SUSPEND_PENDING = 5,
    SBI_STATE_RESUME_PENDING  = 6,
};

enum sbifeat
{
    SBI_FEAT_MISALIGNED_EXC_DELEG  = 0x0,
    SBI_FEAT_LANDING_PAD           = 0x1,
    SBI_FEAT_SHADOW_STACK          = 0x2,
    SBI_FEAT_DOUBLE_TRAP           = 0x3,
    SBI_FEAT_PTE_AD_HW_UPDATING    = 0x4,
    SBI_FEAT_POINTER_MASKING_PMLEN = 0x5,
};

enum sbiresettype
{
    SBI_RESET_SHUTDOWN    = 0x0,
    SBI_RESET_COLD_REBOOT = 0x1,
    SBI_RESET_WARM_REBOOT = 0x2,
};

enum sbireason
{
    SBI_RESET_NO_REASON      = 0x0,
    SBI_RESET_SYSTEM_FAILURE = 0x1
};

uint32 sbi_get_spec_version();

uint64 sbi_get_impl_id();

char *sbi_get_impl_str();

uint64 sbi_get_impl_version();

uint64 sbi_probe_extension(enum sbiext extension_id);

uint64 sbi_get_mvendorid();

uint64 sbi_get_marchid();

uint64 sbi_get_mimpid();

void sbi_set_timer(uint64 stime_value);

struct sbiret sbi_debug_console_write(uint64 num_bytes, void *addr);

struct sbiret sbi_hart_start(uint64 hartid, uint64 addr, uint64 opaque);

struct sbiret sbi_hart_stop();

struct sbiret sbi_hart_get_status(uint64 hartid);

struct sbiret sbi_fwft_set(enum sbifeat feature, uint64 value, uint64 flags);

struct sbiret sbi_fwft_get(enum sbifeat feature);

struct sbiret sbi_system_reset(enum sbiresettype type, enum sbireason reason);

#endif // OS_KERNEL_SBI
