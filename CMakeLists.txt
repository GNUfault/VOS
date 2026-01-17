cmake_minimum_required(VERSION 3.15)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

set(CMAKE_C_COMPILER riscv64-linux-gnu-gcc)
set(CMAKE_ASM_COMPILER riscv64-linux-gnu-gcc)
set(CMAKE_OBJCOPY riscv64-linux-gnu-objcopy)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_ASM_COMPILER_WORKS 1)

project(riscv_kernel C ASM)

set(CMAKE_C_FLAGS "-march=rv64gc -mabi=lp64d -mcmodel=medany -Wall -Wextra -O2 -fno-builtin -nostdlib -ffreestanding")
set(CMAKE_ASM_FLAGS "-march=rv64gc -mabi=lp64d -mcmodel=medany")

set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker.ld -nostdlib -static")

set(CMAKE_C_STANDARD_LIBRARIES "")
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES "")

set(SOURCES
    boot.S
    trap.S
    main.c
    printk.c
    trap.c
    syscall.c
    usermode.c
    process.c
    fs.c
)

add_executable(kernel.elf ${SOURCES})

add_custom_command(
    TARGET kernel.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary kernel.elf kernel.bin
    COMMENT "Creating kernel.bin"
)

add_custom_command(
    TARGET kernel.elf POST_BUILD
    COMMAND riscv64-linux-gnu-size kernel.elf
    COMMENT "Kernel size:"
)

add_custom_target(run
    COMMAND qemu-system-riscv64 -machine virt -bios default -kernel kernel.elf -nographic
    DEPENDS kernel.elf
    COMMENT "Running kernel in QEMU (Ctrl+A then X to exit)"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
