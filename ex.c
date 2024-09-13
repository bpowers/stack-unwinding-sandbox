#include <stdint.h>

volatile uint8_t storage = 0;

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

