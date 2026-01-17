#include "printk.h"
#include "trap.h"
#include "usermode.h"
#include "process.h"
#include "fs.h"

void kmain(void) {
    printk("                ,----..               \n");
    printk("               /   /   \\   .--.--.    \n");
    printk("       ,---.  /   .     : /  /    '.  \n");
    printk("      /__./| .   /   ;.  \\  :  /`. /  \n");
    printk(" ,---.;  ; |.   ;   /  ` ;  |  |--`   \n");
    printk("/___/ \\  | |;   |  ; \\ ; |  :  ;_     \n");
    printk("\\   ;  \\ ' ||   :  | ; | '\\  \\    `.  \n");
    printk(" \\   \\  \\: |.   |  ' ' ' : `----.   \\ \n");
    printk("  ;   \\  ' .'   ;  \\; /  | __ \\  \\  | \n");
    printk("   \\   \\   ' \\   \\  ',  / /  /`--'  / \n");
    printk("    \\   `  ;  ;   :    / '--'.     /  \n");
    printk("     :   \\ |   \\   \\ .'    `--'---'   \n");
    printk("      '---\"     `---`                 \n");

    uint64_t sstatus;
    asm volatile("csrr %0, sstatus" : "=r"(sstatus));
    printk("sstatus: 0x%lx\n", sstatus);
    
    uint64_t satp;
    asm volatile("csrr %0, satp" : "=r"(satp));
    printk("satp (before): 0x%lx\n", satp);
    
    process_init();
    fs_init();
    trap_init();
    printk("Kernel initialization complete!\n");
    printk("Launching POSIX Compliance Test\n");
    printk("\n");
    start_usermode();
    printk("Entering idle loop...\n");
    
    while (1) {
        asm volatile("wfi");
    }
}
