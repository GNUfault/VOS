#include "paging.h"
#include "printk.h"

extern char __page_tables_start[];
extern char __page_tables_end[];

static uint64_t *l2_table = (uint64_t *)__page_tables_start;
static uint64_t *l1_table_0 = (uint64_t *)(__page_tables_start + 0x1000);
static uint64_t *l1_table_2 = (uint64_t *)(__page_tables_start + 0x2000);

// Create a page table entry (PPN goes in bits [53:10])
static inline uint64_t make_pte(uint64_t pa, uint64_t flags) {
    uint64_t pte = ((pa >> 12) << 10) | flags;
    printk("    make_pte(pa=0x%lx, flags=0x%lx) = 0x%lx\n", pa, flags, pte);
    printk("      PPN = 0x%lx (bits [53:10])\n", (pa >> 12));
    return pte;
}

// Get SATP value for Sv39 mode
static inline uint64_t make_satp(uint64_t page_table_pa) {
    uint64_t satp = (8ULL << 60) | (page_table_pa >> 12);
    printk("    make_satp(pt_pa=0x%lx) = 0x%lx\n", page_table_pa, satp);
    printk("      MODE = 8 (Sv39)\n");
    printk("      PPN = 0x%lx\n", (page_table_pa >> 12));
    return satp;
}

void paging_init(void) {
    printk("========================================\n");
    printk("Initializing Sv39 paging (VERBOSE MODE)\n");
    printk("========================================\n\n");
    
    // Check addresses
    printk("Page table addresses:\n");
    printk("  L2 (root): 0x%lx\n", (uint64_t)l2_table);
    printk("  L1_0:      0x%lx\n", (uint64_t)l1_table_0);
    printk("  L1_2:      0x%lx\n", (uint64_t)l1_table_2);
    
    // Verify 4KB alignment
    if (((uint64_t)l2_table & 0xFFF) != 0) {
        printk("ERROR: L2 table not 4KB aligned!\n");
        return;
    }
    if (((uint64_t)l1_table_0 & 0xFFF) != 0) {
        printk("ERROR: L1_0 table not 4KB aligned!\n");
        return;
    }
    if (((uint64_t)l1_table_2 & 0xFFF) != 0) {
        printk("ERROR: L1_2 table not 4KB aligned!\n");
        return;
    }
    printk("  All tables are 4KB aligned - OK!\n\n");
    
    // Clear all page tables
    printk("Clearing page tables...\n");
    for (int i = 0; i < 512; i++) {
        l2_table[i] = 0;
        l1_table_0[i] = 0;
        l1_table_2[i] = 0;
    }
    printk("  Cleared 3 page tables (1536 PTEs)\n\n");
    
    // Set up L2 to point to L1 tables
    // NON-LEAF PTEs have only V bit set (R=W=X=0)
    printk("Setting up L2 (root) page table...\n");
    uint64_t l1_flags = PTE_V;  // Only valid bit, R=W=X=0 means non-leaf
    
    printk("  L2[0] -> L1_0 (for VA 0x00000000-0x3FFFFFFF):\n");
    l2_table[0] = make_pte((uint64_t)l1_table_0, l1_flags);
    
    printk("  L2[2] -> L1_2 (for VA 0x80000000-0xBFFFFFFF):\n");
    l2_table[2] = make_pte((uint64_t)l1_table_2, l1_flags);
    
    printk("\n");
    
    // Fill L1 tables with 2MB megapages (LEAF entries)
    // LEAF PTEs must have at least one of R,W,X set
    printk("Setting up L1 page tables with 2MB megapages...\n");
    uint64_t leaf_flags = PTE_V | PTE_R | PTE_W | PTE_X | PTE_U | PTE_A | PTE_D;
    printk("  Leaf PTE flags: V R W X U A D = 0x%lx\n\n", leaf_flags);
    
    // Map first 1GB with 2MB pages
    printk("  Mapping 0x00000000-0x3FFFFFFF (first 1GB):\n");
    for (int i = 0; i < 512; i++) {
        uint64_t pa = (uint64_t)i * 0x200000;  // 2MB increments
        l1_table_0[i] = make_pte(pa, leaf_flags);
        
        // Only print first few and last to avoid spam
        if (i < 2 || i == 511) {
            uint64_t va_start = (uint64_t)i * 0x200000;
            uint64_t va_end = va_start + 0x200000 - 1;
            printk("    L1_0[%d]: VA 0x%lx-0x%lx -> PA 0x%lx-0x%lx\n",
                   i, va_start, va_end, pa, pa + 0x200000 - 1);
        } else if (i == 2) {
            printk("    ... (508 more entries) ...\n");
        }
    }
    printk("\n");
    
    // Map kernel region (2GB-3GB) with 2MB pages
    printk("  Mapping 0x80000000-0xBFFFFFFF (kernel region, 2GB-3GB):\n");
    for (int i = 0; i < 512; i++) {
        uint64_t pa = 0x80000000ULL + (uint64_t)i * 0x200000;
        l1_table_2[i] = make_pte(pa, leaf_flags);
        
        if (i < 2 || i == 511) {
            uint64_t va_start = 0x80000000ULL + (uint64_t)i * 0x200000;
            uint64_t va_end = va_start + 0x200000 - 1;
            printk("    L1_2[%d]: VA 0x%lx-0x%lx -> PA 0x%lx-0x%lx\n",
                   i, va_start, va_end, pa, pa + 0x200000 - 1);
        } else if (i == 2) {
            printk("    ... (508 more entries) ...\n");
        }
    }
    printk("\n");
    
    // Verify critical mappings
    printk("Verifying critical PTEs:\n");
    printk("  L2[0]   = 0x%lx (should point to L1_0 = 0x%lx)\n", 
           l2_table[0], (uint64_t)l1_table_0);
    printk("  L2[2]   = 0x%lx (should point to L1_2 = 0x%lx)\n",
           l2_table[2], (uint64_t)l1_table_2);
    printk("  L1_2[1] = 0x%lx (should map 0x80200000)\n", l1_table_2[1]);
    printk("\n");
    
    // Calculate current PC location
    uint64_t current_pc;
    asm volatile("auipc %0, 0" : "=r"(current_pc));
    printk("Current PC: 0x%lx\n", current_pc);
    printk("  This is in the 0x80200000 region\n");
    printk("  After paging: VA 0x%lx maps to PA 0x%lx (identity mapped)\n\n",
           current_pc, current_pc);
    
    // Create SATP value
    uint64_t page_table_pa = (uint64_t)l2_table;
    uint64_t satp_val = make_satp(page_table_pa);
    printk("\n");
    
    printk("========================================\n");
    printk("ENABLING PAGING NOW!\n");
    printk("========================================\n");
    printk("Writing SATP = 0x%lx\n", satp_val);
    
    // The CRITICAL sequence: write SATP then SFENCE.VMA
    // Based on RISC-V spec and xv6 implementation
    asm volatile(
        "csrw satp, %0\n"           // Write new SATP
        "sfence.vma zero, zero\n"   // Flush ALL TLB entries
        :: "r"(satp_val)
        : "memory"
    );
    
    printk("SATP written and TLB flushed!\n");
    printk("========================================\n\n");
    
    // Verify it worked
    uint64_t satp_readback;
    asm volatile("csrr %0, satp" : "=r"(satp_readback));
    printk("SATP readback: 0x%lx\n", satp_readback);
    
    if (satp_readback == satp_val) {
        printk("SUCCESS: Paging is ENABLED!\n");
    } else {
        printk("WARNING: SATP value mismatch!\n");
    }
    
    printk("========================================\n");
    printk("Sv39 paging initialization complete!\n");
    printk("========================================\n");
}
