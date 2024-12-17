#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Interpret scause CSR values.\n");
        fprintf(stderr, "Usage: rvscause <scause>.\n");
        return 1;
    }

    uint64_t scause = (uint64_t)strtol(argv[1], NULL, 16);

    if (scause & 1L << 63) {
        switch (scause) {
            case 1:
                printf("Supervisor interrupt\n");
                break;
            case 5:
                printf("Supervisor timer interrupt\n");
                break;
            case 9:
                printf("Supervisor external interrupt\n");
                break;
            case 13:
                printf("Counter-overflow interrupt\n");
                break;
        }
    } else {
        switch (scause) {
            case 0:
                printf("Instruction address misaligned\n");
                break;
            case 1:
                printf("Instruction access fault\n");
                break;
            case 2:
                printf("Illegal instruction\n");
                break;
            case 3:
                printf("Breakpoint\n");
                break;
            case 4:
                printf("Load address misaligned\n");
                break;
            case 5:
                printf("Load access fault\n");
                break;
            case 6:
                printf("Store/AMO address misaligned\n");
                break;
            case 7:
                printf("Store/AMO access fault\n");
                break;
            case 8:
                printf("Environment call from U-mode\n");
                break;
            case 9:
                printf("Environment call from S-mode\n");
                break;
            case 12:
                printf("Instruction page fault\n");
                break;
            case 13:
                printf("Load page fault\n");
                break;
            case 15:
                printf("Store/AMO page fault\n");
                break;
            case 18:
                printf("Software check\n");
                break;
            case 19:
                printf("Hardware error\n");
                break;
        }
    }
}
