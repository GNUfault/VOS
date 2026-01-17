OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
    . = 0x80200000;
    
    .text : {
        *(.text.boot)
        *(.text*)
    }
    
    .rodata : {
        *(.rodata*)
    }
    
    .data : {
        *(.data*)
    }
    
    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }
    
    /* Page table area (aligned to 4KB) */
    . = ALIGN(4096);
    __page_tables_start = .;
    . += 0x10000;  /* 64KB for page tables (multiple levels) */
    __page_tables_end = .;
    
    __kernel_end = .;
}
