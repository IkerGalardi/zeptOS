#ifndef DTB_H
#define DTB_H

#include "types.h"

extern uint64 ram_start;
extern uint64 ram_size;

void dtbparse(void *fdt);

#endif // DTB_H
