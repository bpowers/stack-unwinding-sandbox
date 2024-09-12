#include <stdint.h>
#include <stdbool.h>

#include "printf.h"

#define CORTEX_M3_EXCEPTIONS 16

extern uint32_t _flash_sdata;
extern uint32_t _sram_sdata;
extern uint32_t _sram_edata;
extern uint32_t _sram_sbss;
extern uint32_t _sram_ebss;
extern uint32_t _sram_stacktop;

extern void main(void);
extern void uart0_print(const char *msg);

typedef struct __attribute__((packed)) {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t pc;
  uint32_t xpsr;
} ExceptionFrame;

/* This is an unused handler that simply loops infinitely
 * using the __attribute__ ((weak, alias("function_name")))
 * we'll point all hitherto unused exception handlers to execute
 * this function
 */
void _Unused_Handler(void) {
  uart0_print("in unused handler\n");
  while (1)
    ;
}

/* Declaration of exception handlers
 * based on the exception table
 * Refer: http://www.ti.com/lit/ds/symlink/lm3s6965.pdf Table 2-8
 */
void _Reset_Handler(void);
void _NMI_Handler(void) __attribute__((weak, alias("_Unused_Handler")));
void _Hard_Fault_Handler(void);
void _Memory_Mgmt_Handler(void) __attribute__((weak, alias("_Unused_Handler")));
void _Bus_Fault_Handler(void);
void _Usage_Fault_Handler(void) __attribute__((weak, alias("_Unused_Handler")));
void _SVCall_Handler(void) __attribute__((weak, alias("_Unused_Handler")));
void _Debug_Monitor_Handler(void) __attribute__((weak, alias("_Unused_Handler")));
void _PendSV_Handler(void) __attribute__((weak, alias("_Unused_Handler")));
void _SysTick_Handler(void) __attribute__((weak, alias("_Unused_Handler")));

/* Exception Table */
__attribute__((section(".vectors"), used)) void (*const _exceptions[CORTEX_M3_EXCEPTIONS])(void) = {
    (void (*)(void))((uint32_t)&_sram_stacktop),  // 00: Reset value of the Main Stack Pointer
    _Reset_Handler,                               // 01: Reset value of the Program Counter
    _NMI_Handler,                                 // 02: Non-Maskable Interrupt (NMI)
    _Hard_Fault_Handler,                          // 03: Hard Fault
    _Memory_Mgmt_Handler,                         // 04: Memory Management Fault
    _Bus_Fault_Handler,                           // 05: Bus Fault
    _Usage_Fault_Handler,                         // 06: Usage Fault
    _Unused_Handler,                              // 07: --
    _Unused_Handler,                              // 08: --
    _Unused_Handler,                              // 09: --
    _Unused_Handler,                              // 10: --
    _SVCall_Handler,                              // 11: Supervisor Call
    _Debug_Monitor_Handler,                       // 12: Debug Monitor
    _Unused_Handler,                              // 13: --
    _PendSV_Handler,                              // 14: Pendable req serv
    _SysTick_Handler,                             // 15: System timer tick
};

void blorp(void) {
  printf("we blorpin'\n");
  while (1);
}

__attribute__((section(".startup"))) void __Hard_Fault_Handler(ExceptionFrame *frame, uint32_t reason) {
  printf("got hard fault! (%d) pc %p / %x\n", 7, frame->pc, reason);


  volatile uint32_t *cfsr = (volatile uint32_t *)0xE000ED28;
  const uint32_t usage_fault_mask = 0xffff0000;
  const bool non_usage_fault_occurred = (*cfsr & ~usage_fault_mask) != 0;
  // the bottom 8 bits of the xpsr hold the exception number of the
  // executing exception or 0 if the processor is in Thread mode
  const bool faulted_from_exception = ((frame->xpsr & 0xFF) != 0);

  if (faulted_from_exception || non_usage_fault_occurred) {
    printf("we're going to reboot!\n");
    // For any fault within an ISR or non-usage faults let's reboot the system
    volatile uint32_t *aircr = (volatile uint32_t *)0xE000ED0C;
    *aircr = (0x05FA << 16) | 0x1 << 2;
    while (1) { } // should be unreachable
  }

  // If it's just a usage fault, let's "recover"
  // Clear any faults from the CFSR
  *cfsr |= *cfsr;
  // the instruction we will return to when we exit from the exception
  frame->pc = (uint32_t)blorp;
  // the function we are returning to should never branch
  // so set lr to a pattern that would fault if it did
  frame->lr = 0xdeadbeef;
  // reset the psr state and only leave the
  // "thumb instruction interworking" bit set
  frame->xpsr = (1 << 24);
}

#define HARDFAULT_HANDLING_ASM(_x) \
  __asm volatile(                  \
      "tst lr, #4 \n"              \
      "ite eq \n"                  \
      "mrseq r0, msp \n"           \
      "mrsne r0, psp \n"           \
      "b __Hard_Fault_Handler \n")

__attribute__((section(".startup"))) void _Hard_Fault_Handler(void) {
  HARDFAULT_HANDLING_ASM();
}

__attribute__((section(".startup"))) void _Bus_Fault_Handler() {
  uart0_print("got bus fault\n");
  while (1)
    ;
}

/* _Reset_Handler is what is invoked when the processor is reset.
 * As seen in the vector table, it represents the initial value of
 * the program counter. This is where we setup and call main()
 * We'll create a separate section .startup so this resides
 * immediately after the vector table - as required by LM3S6965 (ARM Cortex-M3)
 */
__attribute__((section(".startup"))) void _Reset_Handler(void) {
  /* Copy the data segment from flash to sram */
  uint32_t *pSrc = &_flash_sdata;
  uint32_t *pDest = &_sram_sdata;

  while (pDest < &_sram_edata) {
    *pDest = *pSrc;
    pDest++;
    pSrc++;
  }

  /* Zero initialize the bss segment in sram */
  pDest = &_sram_sbss;

  while (pDest < &_sram_ebss) {
    *pDest = 0;
    pDest++;
  }

  /* Call main() */
  main();

  /* main() isn't supposed return
   * - if it does, we need to identify this
   * for now, we'll loop infintely
   */
  while (1)
    ;
}
