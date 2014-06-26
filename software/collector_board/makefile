#----------------------------------------------------------
# environment settings
# This is the path to a cloned Arduino GIT repository (which
# contains the arduino libraries)
# For multi user development it is recommended to comment out
# this line and specify ARDUINO_PATH in an Environment Variable
# individually per user system.
#ARDUINO_PATH = /usr/share/Arduino-master
#----------------------------------------------------------
# Project settings
#
#

DIRAVR = /usr
DIRAVRBIN = $(DIRAVR)/bin
DIRAVRUTILS = $(DIRAVR)/utils/bin
DIRINC = $(DIRAVR)/avr/include
DIRLIB = $(DIRAVR)/avr/lib


MCU = atmega88p
F_CPU = 20000000UL

BUILD_DIR = build

TARGET = $(BUILD_DIR)/main

FORMAT = ihex

# Project files
PROJECT_PATH = .
PROJECT_OBJS = main.o usart.o ethernet_twi.o zac.o interpret.o checksum.o config.o
#----------------------------------------------------------
# Programmer settings
# set clock source to external crystal 20MHz:
FUSE_LOW = 0b11110111
# enable burn out detection 4.5V, preserve eeprom on chip erase:
FUSE_HIGH = 0b11010011
AVRDUDE_PROGRAMMER = usbasp
AVRDUDE_PORT = /dev/ttyACM0
AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET)_eeprom.hex
AVRDUDE_WRITE_FUSES = -U hfuse:w:$(FUSE_HIGH) -U lfuse:w:$(FUSE_LOW)
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)
# -F forces programming without checking device
#  identifier.
#  This is, because the arduino bootloader does
#  not conform to the stk500v1 standard when sending
#  the device identifier. Hence the device ID check
#  always will fail.
AVRDUDE_FLAGS += -F
#----------------------------------------------------------
# compiler suite
CC=avr-gcc
CPP=avr-g++
# AR=avr-ar # we currently do not package the libraries in .a files, so this is not needed
OBJCOPY = avr-objcopy
SIZE = avr-size
AVRDUDE = avrdude
#----------------------------------------------------------
# Linker settings
LDFLAGS = -Wl,-Map=$(TARGET).map,--cref
# --gc-sections will allow the linker to remove unused sections
LDFLAGS += -Wl,--gc-sections
#----------------------------------------------------------
# Compiler settings
# -mmcu selects the AVR chip type -> specifies instructionset etc.
# -DF_CPU defines the F_CPU preprocessor makro
# -ffunction-sections -fdata-sections are needed to place
#  functions in separate sections, to be able to remove
#  unneeded functions during linking
C_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Wall -Os -Datmega88p -I$(DIRINC)
#C_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(INCLUDE_FLAGS) -Os
C_FLAGS += -ffunction-sections -fdata-sections
C_FLAGS += -g



# Accumulate all paths to header files
INCLUDE_FLAGS = -I$(ARDUINO_CORE_PATH)
INCLUDE_FLAGS += -I$(ARDUINO_VARIANT_PATH)
INCLUDE_FLAGS += -I$(ETHERNET_LIBRARY_PATH)
INCLUDE_FLAGS += -I$(ETHERNET_UTIL_LIBRARY_PATH)
INCLUDE_FLAGS += -I$(SPI_LIBRARY_PATH)
INCLUDE_FLAGS += -I$(ARDUINO_CORE_LIBC_PATH)
#----------------------------------------------------------

# generate lists of output files (in build dir)
PROJECT_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(PROJECT_OBJS))

#--------------------------------------------------------------
# rules:
all: build size
build: elf hex

# upload the program to the microcontroller flash:
upload:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)
eeprom:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_EEPROM)
fuse:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FUSES)

# analyze compiled program size:
size:
	$(SIZE) $(TARGET).elf

elf: $(TARGET).elf
hex: $(TARGET).hex $(TARGET)_eeprom.hex

# link everything together:
$(TARGET).elf: project
	$(CC) $(C_FLAGS) $(PROJECT_OBJS_FIN) -o $(TARGET).elf $(LDFLAGS)

# convert elf to hex file for programmer:
# exclude eeprom section, in case it exists
# as we do not want to write this to flash
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

# generate eeprom hexfile:
$(TARGET)_eeprom.hex: $(TARGET).elf
	$(OBJCOPY) -O $(FORMAT) -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 $< $@

clean:
	rm -rv $(BUILD_DIR)/*

project: $(PROJECT_OBJS_FIN)

# build Project files
$(PROJECT_OBJS_FIN): %.o: $(patsubst %.o,%.c,$(PROJECT_OBJS))
	$(CC) -c $(C_FLAGS) $(PROJECT_PATH)/$(subst $(BUILD_DIR)/,,$*).c -o $*.o
