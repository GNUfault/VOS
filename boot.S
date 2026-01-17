.section .text.boot
.global _start

_start:
    csrw sie, zero
    
    la sp, stack_top

    la t0, __bss_start
    la t1, __bss_end
clear_bss:
    bgeu t0, t1, bss_done
    sd zero, 0(t0)
    addi t0, t0, 8
    j clear_bss

bss_done:
    call kmain

halt:
    wfi
    j halt

.section .bss
.align 12
.global stack_bottom
stack_bottom:
    .skip 16384
.global stack_top
stack_top:
