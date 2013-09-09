##
## This file is part of the oggbox project.
##
## Copyright (C) 2012 Nathan Dumont <nathan@nathandumont.com>
##
## Oggbox is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Oggbox is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with Oggbox.  If not, see <http://www.gnu.org/licenses/>.
##

BINARY		= oggbox
PREFIX		?= arm-none-eabi
CC		= $(PREFIX)-gcc
LD		= $(PREFIX)-gcc
OBJCOPY		= $(PREFIX)-objcopy
OBJDUMP		= $(PREFIX)-objdump
TOOLCHAIN_DIR	= /usr/$(PREFIX)
CFLAGS		+= -Os -g -Wall -Wextra -I$(TOOLCHAIN_DIR)/include -Ilibopencm3-freertos/include \
		   -IFreeRTOS/Source/include -I./ -IFreeRTOS/Source/portable/GCC/ARM_CM3\
		   -fno-common -mcpu=cortex-m3 -mthumb -msoft-float -MD -DSTM32F1
LDSCRIPT	?= $(BINARY).ld
LDFLAGS		+= -Wl,--start-group -lc -lgcc -Wl,--end-group \
		   -L$(TOOLCHAIN_DIR)/lib -Llibopencm3-freertos/lib \
		   -T$(LDSCRIPT) -nostartfiles -Wl,--gc-sections \
		   -mthumb -march=armv7 -mfix-cortex-m3-ldrd -msoft-float
OBJS		+= main.o vs1053.o block_sd.o fat.o syscalls.o partition.o nd_usart.o \
		   power.o interface.o screen.o font.o font2.o usb.o ogg_meta.o \
		   playlist.o dirent.o handler.o \
		   list.o queue.o tasks.o port.o heap_1.o

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
NULL := 2>/dev/null
else
LDFLAGS += -Wl,--print-gc-sections
endif

.SUFFIXES: .elf .bin .hex .srec .list .images
.SECONDEXPANSION:
.SECONDARY:

all: images

images: $(BINARY).images

handler.o: handler.s
	$(Q)arm-none-eabi-as -o handler.o -c handler.s

list.o: FreeRTOS/Source/list.c
	$(Q)$(CC) $(CFLAGS) -o list.o -c FreeRTOS/Source/list.c

port.o: FreeRTOS/Source/portable/GCC/ARM_CM3/port.c
	$(Q)$(CC) $(CFLAGS) -o port.o -c FreeRTOS/Source/portable/GCC/ARM_CM3/port.c

queue.o: FreeRTOS/Source/queue.c
	$(Q)$(CC) $(CFLAGS) -o queue.o -c FreeRTOS/Source/queue.c

tasks.o: FreeRTOS/Source/tasks.c
	$(Q)$(CC) $(CFLAGS) -o tasks.o -c FreeRTOS/Source/tasks.c

heap_1.o: FreeRTOS/Source/portable/MemMang/heap_1.c
	$(Q)$(CC) $(CFLAGS) -o heap_1.o -c FreeRTOS/Source/portable/MemMang/heap_1.c

%.images: %.bin %.hex %.srec %.list
	@#echo "*** $* images generated ***"

%.bin: %.elf
	@#printf "  OBJCOPY $(*).bin\n"
	$(Q)$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.hex: %.elf
	@#printf "  OBJCOPY $(*).hex\n"
	$(Q)$(OBJCOPY) -Oihex $(*).elf $(*).hex

%.srec: %.elf
	@#printf "  OBJCOPY $(*).srec\n"
	$(Q)$(OBJCOPY) -Osrec $(*).elf $(*).srec

%.list: %.elf
	@#printf "  OBJDUMP $(*).list\n"
	$(Q)$(OBJDUMP) -S $(*).elf > $(*).list

oggbox.elf: $(OBJS) $(LDSCRIPT) libopencm3-freertos/lib/libopencm3_stm32f1.a
	@#printf "  LD      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(LD) -o oggbox.elf $(OBJS) -lopencm3_stm32f1 $(LDFLAGS)

%.o: %.c Makefile
	@#printf "  CC      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(Q)rm -f *.o
	$(Q)rm -f *.d
	$(Q)rm -f *.elf
	$(Q)rm -f *.bin
	$(Q)rm -f *.hex
	$(Q)rm -f *.srec
	$(Q)rm -f *.list

# always uses the same serial adapter no matter what order it was plugged in
install: $(BINARY).hex
	stm32flash -w $(BINARY).hex -v -g 0x0 /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A3000I6Y-if00-port0
.PHONY: images clean

