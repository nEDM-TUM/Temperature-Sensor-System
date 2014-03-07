#
# generic AVR makefile
#
# (c)  July 2011 - Manuel Odendahl - wesen@ruinwesen.com
#

# include this into your main Makefile, after having defined TARGET and TARGET_OBJS

all: $(TARGET).hex

CURDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(CURDIR)MidiCtrl.mk

CC             = avr-gcc
CXX            = avr-g++
OBJCOPY        = avr-objcopy
AVR_ARCH       ?= atmega64
LDAVR_ARCH     ?= avrmega64
FLASH_PROTOCOL = jtag2

CFLAGS   += -Os -ffunction-sections -DAVR -I. -mmcu=$(AVR_ARCH) -mcall-prologues -fshort-enums -fpack-struct -Wall -Werror
CFLAGS   += -Wall -DLITTLE_ENDIAN -g -flto

CFLAGS += no-tree-loop-optimize -ffreestanding -morder1 -funsigned-char -funsigned-bitfields -fshort-enums -fpack-struct
CFLAGS += -fdata-sections -fno-split-wide-types -fno-inline-small-functions -mcall-prologues

CLDFLAGS += -Wl,--relax,--gc-sections -ffunction-sections
CLDFLAGS += -mmcu=$(AVR_ARCH)
LDFLAGS  = -m $(LDAVR_ARCH) -M

# generate list
# CFLAGS += -Wa,-adhlns=$@.lst

%.o: %.cpp
        $(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
        $(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
        $(CC) $(CFLAGS) -c $< -o $@

%.s: %.c
        $(CC) -S $(CFLAGS) -fverbose-asm $< -o $@

%.o: %.S
        $(CC) $(CFLAGS) -c $< -o $@

%.syx: %.hex
        ihex2sysex $< $@

%.srec: %.elf
        $(OBJCOPY) -j .text -j .data -O srec $< $@

%.hex: %.elf
        $(OBJCOPY) -j .text -j .data -O ihex $< $@

%.ee_srec: %.elf
        $(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@

AVR_BASE_DIR  ?= $(abspath $(CURDIR)..)
AVR_LIB_DIR   ?= $(AVR_BASE_DIR)/hardware/libraries

AVR_LIBS       += CommonTools Midi
AVR_LIB_DIRS   += $(foreach lib,$(AVR_LIBS),$(AVR_LIB_DIR)/$(lib))
AVR_INC_FLAGS  += $(foreach dir,$(AVR_LIB_DIRS),-I$(dir))
AVR_OBJS       += $(foreach dir,$(AVR_LIB_DIRS),$(foreach file,$(wildcard $(dir)/*.cpp),$(subst .cpp,.o,$(file))))
AVR_OBJS       += $(foreach dir,$(AVR_LIB_DIRS),$(foreach file,$(filter-out $(AVR_HOST_EXCLUDE),$(wildcard $(dir)/*.c)),$(subst .c,.o,$(file))))
AVR_DEPS       += $(subst .o,.d,$(AVR_OBJS))

# AVR_HOST_EXCLUDE can be used to exclude specific files later on

CXXFLAGS += $(AVR_INC_FLAGS)
CFLAGS += $(AVR_INC_FLAGS)

CXXFlags += -Werror -Wall
CFLAGS += -Werror -Wall

default: all

%.d:%.c
        set -e; $(CC) -MM $(CFLAGS) $< \
        | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@ ; \
        [ -s $@ ] || rm -f $@

%.d:%.cpp
        set -e; $(CXX) -MM $(CXXFLAGS) $< \
        | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@ ; \
        [ -s $@ ] || rm -f $@

%.host.d:%.c
        set -e; $(CC) -MM $(CFLAGS) $< \
        | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@ ; \
        [ -s $@ ] || rm -f $@

%.host.d:%.cpp
        set -e; $(CXX) -MM $(CXXFLAGS) $< \
        | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@ ; \
        [ -s $@ ] || rm -f $@

printlibs:
        echo $(AVR_LIBS)

$(TARGET).elf: $(TARGET).o $(TARGET_OBJS) $(AVR_OBJS)
        $(CXX) $(CLDFLAGS) -g -o $@ $^

_clean:
        - rm *.elf *.hex *.o .midictrl.flags

libclean:
        rm -rf $(TARGET_OBJS) $(OBJS)

# concrete settings for development environment

UNAME=$(shell uname)
ISWIN=$(findstring CYGWIN,$(UNAME))
ISMAC=$(findstring Darwin,$(UNAME))

CC = avr-gcc
CXX = avr-g++
OBJCOPY = avr-objcopy
AVR_ARCH = atmega64
F_CPU = 16000000L
CORE = minicommand2
