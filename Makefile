
CC = arm-none-eabi-gcc
CFLAGS = -g -mcpu=cortex-m3 -mthumb -fno-omit-frame-pointer -mtpcs-frame -mtpcs-leaf-frame -mapcs-frame \
  -DPRINTF_DISABLE_SUPPORT_FLOAT=1 -DPRINTF_DISABLE_SUPPORT_LONG_LONG=1

CONFIG = Makefile

# quiet output, but allow us to look at what commands are being
# executed by passing 'V=1' to make, without requiring temporarily
# editing the Makefile.
ifneq ($V, 1)
MAKEFLAGS += -s
endif

.SUFFIXES:
.SUFFIXES: .c .o .test

all: system.bin

.c.o: $(CONFIG)
	@echo "  CC    $@"
	$(CC) $(CFLAGS) -MMD -o $@ -c $<


startup_lm3s6965.elf: startup_lm3s6965.o main.o printf.o
	arm-none-eabi-ld -T lm3s6965_layout.ld -o $@ $^

system.bin: startup_lm3s6965.elf
	arm-none-eabi-objcopy -O binary $< $@

run: system.bin
	qemu-system-arm -M lm3s6965evb -kernel system.bin -nographic -monitor telnet:127.0.0.1:1234,server,nowait 

rundbg: system.bin
	qemu-system-arm -S -M lm3s6965evb -kernel system.bin -gdb tcp::5678 -nographic -monitor telnet:127.0.0.1:1234,server,nowait 

clean:
	rm -f *.o *.elf *.bin *.d

format:
	clang-format -i *.c *.h

dump: 
	arm-none-eabi-nm -n startup_lm3s6965.o
	arm-none-eabi-objdump -h startup_lm3s6965.o
	arm-none-eabi-nm -n serial_print.o
	arm-none-eabi-objdump -h serial_print.o
	arm-none-eabi-nm -n startup_lm3s6965.elf
	arm-none-eabi-objdump -h startup_lm3s6965.elf
	arm-none-eabi-size startup_lm3s6965.elf

