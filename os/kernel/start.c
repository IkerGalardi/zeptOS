#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

void main();
void main_secondary();
void timerinit();

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

int is_start_core = 1;

// a scratch area per CPU for machine-mode timer interrupts.
uint64 timer_scratch[NCPU][5];

// assembly code in kernelvec.S for machine-mode timer interrupt.
extern void timervec();

// entry.S jumps here in machine mode on stack0.
void
start(uint64 hartid, void *fdt)
{
    // disable paging for now.
    w_satp(0);

    // enable interrupts
    w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

    // TODO: ask for clock interrupts.

    // keep each CPU's hartid in its tp register, for cpuid().
    w_tp(hartid);

    // call the main function
    __sync_synchronize();
    if (started == 0) {
        // next calls to this function go to main_secondary() instead of main()
        started = 1;

        main();
    } else {
        main_secondary();
    }
}
