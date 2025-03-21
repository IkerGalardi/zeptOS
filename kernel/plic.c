#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "globals.h"
#include "riscv.h"
#include "defs.h"

//
// the riscv Platform Level Interrupt Controller (PLIC).
//

void
plicinit(void)
{
    // set desired IRQ priorities non-zero (otherwise disabled).
    *(uint32*)(plic + uart0_irq*4) = 1;
    *(uint32*)(plic + virtio0_irq*4) = 1;
}

void
plicinithart(void)
{
    int hart = cpuid();

    // set enable bits for this hart's S-mode
    // for the uart and virtio disk.
    *(uint32*)PLIC_SENABLE(hart) = (1 << uart0_irq) | (1 << virtio0_irq);

    // set this hart's S-mode priority threshold to 0.
    *(uint32*)PLIC_SPRIORITY(hart) = 0;
}

// ask the PLIC what interrupt we should serve.
int
plic_claim(void)
{
    int hart = cpuid();
    int irq = *(uint32*)PLIC_SCLAIM(hart);
    return irq;
}

// tell the PLIC we've served this IRQ.
void
plic_complete(int irq)
{
    int hart = cpuid();
    *(uint32*)PLIC_SCLAIM(hart) = irq;
}
