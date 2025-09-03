#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "sbi.h"

uint64 sys_shutdown()
{
    if (sbi_probe_extension(SBI_EXTENSION_SRST) == 0) {
        printf("kernel(%d): system reset extension not supported\n", cpuid());
        return -1;
    }

    struct sbiret res = sbi_system_reset(SBI_RESET_SHUTDOWN, SBI_RESET_NO_REASON);
    if (res.error != SBI_SUCCESS) {
        printf("kernel(%d): shutdown failed: %s\n", cpuid(), sbi_error_str(res.error));
        return -1;
    }

    return 0;
}
