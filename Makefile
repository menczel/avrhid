# Makefile for creating AVR USB HID devices
# Target: Arduino Leonardo and Micro (Atmega32u4)
# Requires: Linux OS with avr-gcc installed
# Copyright (C) 2017 Laszlo Menczel
# laszlo.menczel@gmail.com

PLAT = $(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')
  
#=======================================================================

ifeq ($(PLAT),linux)
  mnt_dir := $(shell pwd)

  CC := /usr/bin/avr-gcc
  OC := /usr/bin/avr-objcopy
  OS := /usr/bin/avr-size
  RM := rm -f
else
  $(error Unsupported platform)
endif

#=======================================================================

src_dir := $(mnt_dir)/src
bld_dir := $(mnt_dir)/build
obj_dir := $(mnt_dir)/build/obj

#===================== Manual configuration ============================

# Board  type. Uncomment only one!
BOARD_TYPE = ARD_LEONARDO
# BOARD_TYPE = ARD_MICRO

## Device type. Uncomment only one!
# DEV_TYPE = HID_KEYBOARD
DEV_TYPE = HID_MOUSE
# DEV_TYPE = HID_JOYSTICK

## ========== No manually configurable items below this line ===========

## Name of compiled program
ifeq ($(DEV_TYPE),HID_KEYBOARD)
  DEV_NAME = usb_keyboard
endif

ifeq ($(DEV_TYPE),HID_MOUSE)
  DEV_NAME = usb_mouse
endif

ifeq ($(DEV_TYPE),HID_JOYSTICK)
  DEV_NAME = usb_joystick
endif

ifeq ($(BOARD_TYPE),ARD_LEONARDO)
  PROD = 0x8036
else
  PROD = 0x8037
endif

#=======================================================================

CFLAGS = -c -Os -Wall -I$(src_dir) -MMD -mmcu=atmega32u4 \
         -DF_CPU=16000000UL -D__AVR_ATmega32U4__ -DPRODUCT_ID=$(PROD) \
         -D$(DEV_TYPE) -o $@ $<

define DO_CC
@echo "CC $<"
@$(CC) $(CFLAGS)
endef

$(obj_dir)/%.o: $(src_dir)/%.c
	$(DO_CC)

OBJS = $(obj_dir)/hid.o

ifeq ($(DEV_TYPE),HID_KEYBOARD)
  OBJS += $(obj_dir)/hid_kbd.o $(obj_dir)/dev_kbd.o
endif

ifeq ($(DEV_TYPE),HID_MOUSE)
  OBJS += $(obj_dir)/hid_mou.o $(obj_dir)/dev_mou.o
endif

ifeq ($(DEV_TYPE),HID_JOYSTICK)
  OBJS += $(obj_dir)/hid_joy.o $(obj_dir)/dev_joy.o
endif

#=======================================================================

ELF_IMG = $(bld_dir)/$(DEV_NAME).elf
HEX_IMG = $(bld_dir)/$(DEV_NAME).hex
HEX_LEN = $(bld_dir)/$(DEV_NAME).len

define DO_LINK
@echo LINK object files to $(DEV_NAME).elf
@$(CC) -Wall -mmcu=atmega32u4 -DF_CPU=16000000UL $(OBJS) -o $(ELF_IMG)
endef

define DO_OC
@echo OBJCOPY $(DEV_NAME).elf to $(DEV_NAME).hex
@$(OC) -O ihex $(ELF_IMG) $(HEX_IMG)
endef

all: device

device: $(OBJS)
	$(DO_LINK)
	$(DO_OC)
	@$(OS) $(ELF_IMG) 1>$(HEX_LEN)
	$(OS) $(ELF_IMG)

#=======================================================================
# Blink program to restore board to its factory state

BLINK_SRC = $(src_dir)/blink.c
BLINK_ELF = $(bld_dir)/blink.elf
BLINK_HEX = $(bld_dir)/blink.hex

blink: $(BLINK_OBJS)
	$(CC) -O -Wall -mmcu=atmega32u4 -DF_CPU=16000000UL -o $(BLINK_ELF) $(BLINK_SRC)
	$(OC) -O ihex $(BLINK_ELF) $(BLINK_HEX)
	$(OS) $(BLINK_ELF)

#=======================================================================

DEP_FILES = $(filter %.d,$(OBJS:%.o=%.d))

-include $(DEP_FILES)

clean:
	$(RM) $(bld_dir)/*.elf
	$(RM) $(bld_dir)/*.hex
	$(RM) $(bld_dir)/*.len
	$(RM) $(obj_dir)/*.o

xclean:
	$(RM) $(bld_dir)/*.elf
	$(RM) $(bld_dir)/*.hex
	$(RM) $(bld_dir)/*.len
	$(RM) $(obj_dir)/*.o
	$(RM) $(obj_dir)/*.d

.PHONY: all clean xclean
