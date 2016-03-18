BINARY		= main

PREFIX ?= arm-softfloat-elf
RAMSIZE ?= 0x2005000

TOOLCHAIN_DIR   = ../../../arm/libopenstm32
CC=$(PREFIX)-gcc
LD=$(PREFIX)-ld
OBJCOPY=$(PREFIX)-objcopy
OBJDUMP=$(PREFIX)-objdump
CFLAGS		= -O0 -g -Wall -Wextra -I$(TOOLCHAIN_DIR)/include -fno-common \
		  -mcpu=cortex-m3 -mthumb
LDSCRIPT	= $(BINARY).ld
LDFLAGS		= -nostartfiles \
		  -L$(TOOLCHAIN_DIR)/lib \
		  -L/usr/local/cross-arm/arm-softfloat-elf/lib/thumb \
		  -L/usr/local/cross-arm/lib/gcc/arm-softfloat-elf/4.4.3/thumb \
		  -T$(LDSCRIPT) \
		  --defsym=_stack=0x20005000 

OBJS		= $(BINARY).o \
			ob_printf.o	\
			ob_string_case.o	\
			ob_itoa.o	\
			ob_strlen.o	\
			ob_screen.o	\
			font.o

OPENOCD_BASE	= /usr
OPENOCD		= $(OPENOCD_BASE)/bin/openocd
OPENOCD_SCRIPTS	= $(OPENOCD_BASE)/share/openocd/scripts
OPENOCD_FLASHER	= $(OPENOCD_SCRIPTS)/interface/olimex-arm-usb-ocd.cfg
OPENOCD_BOARD	= $(OPENOCD_SCRIPTS)/board/olimex_stm32_h103.cfg

ifneq ($(V),1)
Q := @
NULL := 2>/dev/null
endif

#Sources=startup.c;src/hw_config.c;src/stm3210e_lcd.c;src/stm32f10x_it.c;../../Libraries/CMSIS/Core/CM3/core_cm3.c;

#CFLAGS=-include stm32f10x_conf.h

all: images

images: $(BINARY)
	@printf "  OBJCOPY $(BINARY).bin\n"
	$(Q)$(OBJCOPY) -Obinary $(BINARY) $(BINARY).bin
	@printf "  OBJCOPY $(BINARY).hex\n"
	$(Q)$(OBJCOPY) -Oihex $(BINARY) $(BINARY).hex
	@printf "  OBJCOPY $(BINARY).srec\n"
	$(Q)$(OBJCOPY) -Osrec $(BINARY) $(BINARY).srec
	@printf "  OBJDUMP $(BINARY).list\n"
	$(Q)$(OBJDUMP) -S $(BINARY) > $(BINARY).list

$(BINARY): $(OBJS) $(LDSCRIPT)
	@printf "  LD      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(LD) $(LDFLAGS) -o $(BINARY) $(OBJS) -lopenstm32 -lc -lgcc

%.o: %.c Makefile
	@printf "  CC      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@printf "  CLEAN   $(subst $(shell pwd)/,,$(OBJS))\n"
	$(Q)rm -f *.o
	@printf "  CLEAN   $(BINARY)\n"
	$(Q)rm -f $(BINARY)
	@printf "  CLEAN   $(BINARY).bin\n"
	$(Q)rm -f $(BINARY).bin
	@printf "  CLEAN   $(BINARY).hex\n"
	$(Q)rm -f $(BINARY).hex
	@printf "  CLEAN   $(BINARY).srec\n"
	$(Q)rm -f $(BINARY).srec
	@printf "  CLEAN   $(BINARY).list\n"
	$(Q)rm -f $(BINARY).list

flash: images
	@printf "  FLASH   $(BINARY).bin\n"
	@# IMPORTANT: Don't use "resume", only "reset" will work correctly!
	$(Q)$(OPENOCD) -s $(OPENOCD_SCRIPTS) \
		       -f $(OPENOCD_FLASHER) \
		       -f $(OPENOCD_BOARD) \
		       -c "init" -c "reset halt" \
		       -c "flash write_image erase $(BINARY).hex" \
		       -c "reset" \
		       -c "shutdown" $(NULL)

.PHONY: images clean
