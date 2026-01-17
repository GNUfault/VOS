#ifndef SYSCALL_H
#define SYSCALL_H

#include "trap.h"

void syscall_handler(struct trap_frame *tf);

#endif
