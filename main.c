#include <stdint.h>

#include "printf.h"

/* We'll be using the serial interface to print on the console (qemu)
 * For this, we write to the UART.
 * Refer: http://www.ti.com/lit/ds/symlink/lm3s6965.pdf Table 2-8
 * for the mapping of UART devices (including UART0)
 */

// #define LM3S6965_UART0 (*(volatile uint8_t *)0x4000C000)

volatile uint8_t *lm3s6965_uart0 = (uint8_t *)0x4000C000;

volatile uint8_t storage = 0;

void uart0_print(const char *msg);

void c(void) {
  typedef void (*fn_t)();
  fn_t foo = (fn_t)(0x8004000);
  foo();
}
void b(void) {
  c();
}
void a(void) {
  b();
}

volatile uint32_t *stack_topish;

void main(void) {
  volatile int n = 0;
  stack_topish = &n;

  printf("addr of n %p\n", &n);

  a();

  const char *start_msg = "Hello, World\n";

  uart0_print(start_msg);

  while (1)
    ;
}

void _putchar(char c) {
  *lm3s6965_uart0 = c;
}

void uart0_print(const char *msg) {
  while (*msg) {
    *lm3s6965_uart0 = *msg;
    msg++;
  }
}
