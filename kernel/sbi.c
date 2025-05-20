#include "sbi.h"

static inline struct sbiret ecall(int extension, int function,
                                  uint64 arg0,
                                  uint64 arg1,
                                  uint64 arg2,
                                  uint64 arg3,
                                  uint64 arg4,
                                  uint64 arg5)
{

    register uint64 a0 asm ("a0") = arg0;
    register uint64 a1 asm ("a1") = arg1;
    register uint64 a2 asm ("a2") = arg2;
    register uint64 a3 asm ("a3") = arg3;
    register uint64 a4 asm ("a4") = arg4;
    register uint64 a5 asm ("a5") = arg5;
    register uint64 a6 asm ("a6") = function;
    register uint64 a7 asm ("a7") = extension;
    asm volatile ("ecall"
                : "+r" (a0), "+r" (a1)
                : "r" (a2), "r" (a3), "r" (a4), "r" (a5), "r" (a6), "r" (a7)
                : "memory");

    struct sbiret result = {
        .error = a0,
        .value = a1,
    };
    return result;
}

uint32 sbi_get_spec_version()
{
    return ecall(SBI_EXTENSION_BASE, 0, 0, 0, 0, 0, 0, 0).value;
}

uint64 sbi_get_impl_id()
{
    return ecall(SBI_EXTENSION_BASE, 1, 0, 0, 0, 0, 0, 0).value;
}

char *sbi_get_impl_str()
{
    switch (sbi_get_impl_id()) {
        case 0: return "BBL";
        case 1: return "OpenSBI";
        case 2: return "Xvisor";
        case 3: return "KVM";
        case 4: return "RustSBI";
        case 5: return "Dioxis";
        case 6: return "Coffer";
        case 7: return "Xen";
        case 8: return "PolarFire Hart Software Services";
        default: return "Unknown";
    }
}

uint64 sbi_get_impl_version()
{
    return ecall(SBI_EXTENSION_BASE, 2, 0, 0, 0, 0, 0, 0).value;
}

uint64 sbi_probe_extension(enum sbiext extension_id)
{
    return ecall(SBI_EXTENSION_BASE, 3, extension_id, 0, 0, 0, 0, 0).value;
}

uint64 sbi_get_mvendorid()
{
    return ecall(SBI_EXTENSION_BASE, 4, 0, 0, 0, 0, 0, 0).value;
}

uint64 sbi_get_marchid()
{
    return ecall(SBI_EXTENSION_BASE, 5, 0, 0, 0, 0, 0, 0).value;
}

uint64 sbi_get_mimpid()
{
    return ecall(SBI_EXTENSION_BASE, 6, 0, 0, 0, 0, 0, 0).value;
}

void sbi_set_timer(uint64 stime_value)
{
    ecall(SBI_EXTENSION_TIME, 0, stime_value, 0, 0, 0, 0, 0);
}

struct sbiret sbi_debug_console_write(uint64 num_bytes, void *addr)
{
    uint64 ptr = (uint64)addr;
    uint32 lower = ptr & 0xffffffff;
    uint32 upper = ((ptr) >> 16) >> 16;
    return ecall(SBI_EXTENSION_DBCN, 0, num_bytes, lower, upper, 0, 0, 0);
}

struct sbiret sbi_hart_start(uint64 hartid, uint64 addr, uint64 opaque)
{
    return ecall(SBI_EXTENSION_HSM, 0, hartid, addr, opaque, 0, 0, 0);
}

struct sbiret sbi_hart_stop()
{
    return ecall(SBI_EXTENSION_HSM, 1, 0, 0, 0, 0, 0, 0);
}

struct sbiret sbi_hart_get_status(uint64 hartid)
{
    return ecall(SBI_EXTENSION_HSM, 2, hartid, 0, 0, 0, 0, 0);
}

struct sbiret sbi_fwft_set(enum sbifeat feature, uint64 value, uint64 flags)
{
    return ecall(SBI_EXTENSION_FWFT, 0, feature, value, flags, 0, 0, 0);
}

struct sbiret sbi_fwft_get(enum sbifeat feature)
{
    return ecall(SBI_EXTENSION_FWFT, 0, feature, 0, 0, 0, 0, 0);
}
