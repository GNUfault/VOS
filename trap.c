#include "trap.h"
#include "printk.h"
#include "syscall.h"

extern void trap_vector(void);

void trap_init(void) {
    printk("Initializing trap handlers...\n");
    
    uint64_t tvec = (uint64_t)trap_vector;
    asm volatile("csrw stvec, %0" :: "r"(tvec));
    
    asm volatile("csrsi sstatus, 0x2");
    
    printk("Trap vector at: 0x%lx\n", tvec);
    printk("Traps initialized!\n");
}

void trap_handler(struct trap_frame *tf) {
    uint64_t scause, sepc, stval;
    
    asm volatile("csrr %0, scause" : "=r"(scause));
    asm volatile("csrr %0, sepc" : "=r"(sepc));
    asm volatile("csrr %0, stval" : "=r"(stval));
    
    if (scause & (1ULL << 63)) {
        uint64_t int_num = scause & 0x7FFFFFFFFFFFFFFF;
        printk("Interrupt %lu at PC 0x%lx\n", int_num, sepc);
    } else {
        switch (scause) {
            case 8:
                syscall_handler(tf);
                sepc += 4;
                asm volatile("csrw sepc, %0" :: "r"(sepc));
                break;
                
            case 12:
                printk("Instruction page fault at 0x%lx (addr: 0x%lx)\n", sepc, stval);
                break;
                
            case 13:
                printk("Load page fault at 0x%lx (addr: 0x%lx)\n", sepc, stval);
                break;
                
            case 15:
                printk("Store page fault at 0x%lx (addr: 0x%lx)\n", sepc, stval);
                break;
                
            default:
                printk("Unknown exception %lu at PC 0x%lx\n", scause, sepc);
                printk("stval: 0x%lx\n", stval);
                break;
        }
    }
}

