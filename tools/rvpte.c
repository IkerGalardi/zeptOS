#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Interpret page table entries of the Sv39 virtual memory system.\n");
        fprintf(stderr, "Usage: rvpte <pte>.\n");
        return 1;
    }

    // TODO: detect if its a decimal number or hex (by checking if it starts
    // with 0x or not)
    uint64_t pte = (uint64_t)strtol(argv[1], NULL, 16);

    printf("V = %d\n", (pte & 1) > 0);
    printf("R = %d\n", (pte & (1 << 1)) > 0);
    printf("W = %d\n", (pte & (1 << 2)) > 0);
    printf("X = %d\n", (pte & (1 << 3)) > 0);
    printf("U = %d\n", (pte & (1 << 4)) > 0);
    printf("G = %d\n", (pte & (1 << 5)) > 0);
    printf("A = %d\n", (pte & (1 << 6)) > 0);
    printf("D = %d\n", (pte & (1 << 7)) > 0);

    uint64_t ppn0 = (pte >> 10) & 0x1FF;
    uint64_t ppn1 = (pte >> 19) & 0x1FF;
    uint64_t ppn2 = (pte >> 28) & 0x7FFFFFF;
    uint64_t ppn = ppn0 | (ppn1 << 9) | (ppn2 << 18);

    printf("ppn = %lx (ppn0 = %lx, ppn1 = %lx, ppn2 = %lx)\n", ppn, ppn0, ppn1, ppn2);
}
