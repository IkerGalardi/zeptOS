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
    SBI_EXTENSION_HSM  = 0x4853D,
    SBI_EXTENSION_SRST = 0x53525354,
    SBI_EXTENSION_PMU  = 0x504D55,
    SBI_EXTENSION_DBCN = 0x4442434E,
    SBI_EXTENSION_SUSP = 0x53555350,
    SBI_EXTENSION_CPPC = 0x43505043,
    SBI_EXTENSION_NACL = 0x4E41434C,
    SBI_EXTENSION_STA  = 0x535441,
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

uint32 sbi_get_spec_version();

uint64 sbi_get_impl_id();

char *sbi_get_impl_str();

uint64 sbi_get_impl_version();

uint64 sbi_probe_extension(enum sbiext extension_id);

uint64 sbi_get_mvendorid();

uint64 sbi_get_marchid();

uint64 sbi_get_mimpid();

struct sbiret sbi_debug_console_write(uint64 num_bytes, void *addr);

struct sbiret sbi_hart_start(uint64 hartid, uint64 addr, uint64 opaque);

struct sbiret sbi_hart_stop();

struct sbiret sbi_hart_get_status(uint64 hartid);

#endif // OS_KERNEL_SBI
