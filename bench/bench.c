#include "kernel/types.h"
#include "user.h"

char *ciphertest_argv[] = {"ciphertest", 0};

int main(int argc, char *argv[])
{
    printf("bench: running ciphertest...\n");
    int pid = fork();
    if (pid == 0) {
        exec(ciphertest_argv[0], ciphertest_argv);
    }
    wait(0);
}
