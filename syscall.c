#include "syscall.h"
#include "printk.h"
#include "process.h"
#include "fs.h"

#define SYS_EXIT    1
#define SYS_FORK    2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_WAIT    7
#define SYS_EXEC    8
#define SYS_GETPID  9
#define SYS_KILL    10
#define SYS_PUTCHAR 100

void syscall_handler(struct trap_frame *tf) {
    uint64_t syscall_num = tf->x17;
    uint64_t arg0 = tf->x10;
    uint64_t arg1 = tf->x11;
    uint64_t arg2 = tf->x12;
    
    uint64_t ret = 0;
    
    switch (syscall_num) {
        case SYS_EXIT: {
            process_exit((int)arg0);
            ret = 0;
            break;
        }
        
        case SYS_FORK: {
            ret = process_fork();
            if (ret > 0) {
                process_t *child = process_get(ret);
                if (child) {
                    child->context.regs[10] = 0;
                }
            }
            break;
        }
        
        case SYS_READ: {
            ret = fs_read((int)arg0, (void *)arg1, (uint32_t)arg2);
            break;
        }
        
        case SYS_WRITE: {
            ret = fs_write((int)arg0, (const void *)arg1, (uint32_t)arg2);
            break;
        }
        
        case SYS_OPEN: {
            ret = fs_open((const char *)arg0, (int)arg1);
            break;
        }
        
        case SYS_CLOSE: {
            ret = fs_close((int)arg0);
            break;
        }
        
        case SYS_WAIT: {
            ret = process_wait((int *)arg0);
            break;
        }
        
        case SYS_EXEC: {
            ret = process_exec((const char *)arg0);
            break;
        }
        
        case SYS_GETPID: {
            process_t *proc = process_current();
            ret = proc ? proc->pid : -1;
            break;
        }
        
        case SYS_KILL: {
            ret = process_kill((int)arg0, (int)arg1);
            break;
        }
        
        case SYS_PUTCHAR: {
            printk("%c", (char)arg0);
            ret = 0;
            break;
        }
        
        default:
            printk("%lu: Function not implemented!\n", syscall_num);
            ret = -1;
            break;
    }
   
    tf->x10 = ret;
}
