// init: The initial user-level program

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#ifndef CONFIG_BENCHMARK_ENABLED
char *argv[] = { "sh", 0 };
#else
char *argv[] = { "ciphertest", 0 };
#endif // CONFIG_BENCHMARK_ENABLED

int
main(void)
{
    int pid, wpid;

    if(open("console", O_RDWR) < 0){
        mknod("console", CONSOLE, 0);
        open("console", O_RDWR);
    }
    dup(0);    // stdout
    dup(0);    // stderr

    for(;;){
#ifndef CONFIG_BENCHMARK_ENABLED
        printf("init: starting sh\n");
#else
        printf("init: starting ciphertest\n");
#endif //CONFIG_BENCHMARK_ENABLED
        pid = fork();
        if(pid < 0){
            printf("init: fork failed\n");
            exit(1);
        }
        if(pid == 0){
#ifndef CONFIG_BENCHMARK_ENABLED
            exec("sh", argv);
            printf("init: exec sh failed\n");
            exit(1);
#else
            exec("ciphertest", argv);
            printf("init: exec ciphertest failed\n");
            exit(1);
#endif // CONFIG_BENCHMARK_ENABLED
        }

        for(;;){
            // this call to wait() returns if the shell exits,
            // or if a parentless process exits.
            wpid = wait((int *) 0);
            if(wpid == pid){
#ifndef CONFIG_BENCHMARK_ENABLED
                printf("init: shell exited, restarting it\n");
                break;
#else
                shutdown();
#endif // CONFIG_BENCHMARK_ENABLED
            } else if(wpid < 0){
                printf("init: wait returned an error\n");
                exit(1);
            } else {
                // it was a parentless process; do nothing.
            }
        }
    }
}
