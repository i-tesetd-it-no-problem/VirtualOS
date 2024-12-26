#include "utils/trace.h"
#include <stdint.h>
#include <stdio.h>

void __show_trace(uint32_t* regs)
{
    uint32_t *r0   = &regs[0];
    uint32_t *r1   = &regs[1];
    uint32_t *r2   = &regs[2];
    uint32_t *r3   = &regs[3];
    uint32_t *r12  = &regs[4];
    uint32_t *lr   = &regs[5];
    uint32_t *pc   = &regs[6];
    uint32_t *xpsr = &regs[7];

    printf("PC: 0x%08x\n\r", *pc);

    // arm-none-eabi-addr2line -e xxx.elf `PC`
}

/**
 * @brief 需要编译选项支持
 * set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m4 -mthumb") # 指定特定的架构 例如 cortex-m4
 * 
 */
__attribute__((naked))
void trace_interrupt_exception_entry(void)
{
    __asm volatile (
        "mrs    r0, msp             \n"
        "ldr    r1, = __show_trace  \n"
        "bx     r1                  \n"
    );
}
