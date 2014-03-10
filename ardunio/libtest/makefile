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
MCU = atmega328
F_CPU = 16000000UL

BOARD_TYPE = ethernet

BUILD_DIR = build

TARGET = $(BUILD_DIR)/main

FORMAT = ihex

# Project files
PROJECT_PATH = ./
PROJECT_OBJS = ChatServer.o
#----------------------------------------------------------
# Programmer settings
AVRDUDE_PROGRAMMER = stk500v1
AVRDUDE_PORT = /dev/ttyACM0
AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
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
# -Wl and --gc-sections are needed to avoid __cxa_pure_virtual() reference errors
#  which are caused by arduino c++ virtual functions
LDFLAGS += -Wl,--gc-sections
#----------------------------------------------------------
# Compiler settings
# -mmcu selects the AVR chip type -> specifies instructionset etc.
# -DF_CPU defines the F_CPU preprocessor makro
CPP_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(INCLUDE_FLAGS) -Os
# -ffunction-sections -fdata-sections are needed to avoid
#  __cxa_pure_virtual() reference errors which are caused by
#  arduino c++ virtual functions
CPP_FLAGS += -ffunction-sections -fdata-sections
C_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(INCLUDE_FLAGS) -Os
#----------------------------------------------------------
# Arduino library settings:
# this specifies which part of the Arduino Core library to compile:
# Arduino core library consists of C++ AND C sources
# => handle them separately here
ARDUINO_CORE_PATH = $(ARDUINO_PATH)/hardware/arduino/cores/arduino
ARDUINO_CORE_OBJS = CDC.o HardwareSerial.o HID.o IPAddress.o new.o Print.o Stream.o Tone.o USBCore.o WMath.o WString.o 
ARDUINO_CORE_C_OBJS = WInterrupts.o wiring_analog.o wiring.o wiring_digital.o wiring_pulse.o wiring_shift.o

# this is for board specific headers
ARDUINO_VARIANT_PATH = $(ARDUINO_PATH)/hardware/arduino/variants/$(BOARD_TYPE)

ARDUINO_LIBRARIES_PATH = $(ARDUINO_PATH)/libraries

# Ethernet library:
ETHERNET_LIBRARY_PATH = $(ARDUINO_LIBRARIES_PATH)/Ethernet
ETHERNET_OBJS = Dhcp.o Dns.o EthernetClient.o Ethernet.o EthernetServer.o EthernetUdp.o  
ETHERNET_UTIL_LIBRARY_PATH = $(ARDUINO_LIBRARIES_PATH)/Ethernet/utility
ETHERNET_UTIL_OBJS = socket.o w5100.o

# SPI Library (required by Ethernet Library)
SPI_LIBRARY_PATH = $(ARDUINO_LIBRARIES_PATH)/SPI
SPI_OBJS = SPI.o

# Accumulate all paths to header files
INCLUDE_FLAGS = -I$(ARDUINO_CORE_PATH)
INCLUDE_FLAGS += -I$(ARDUINO_VARIANT_PATH)
INCLUDE_FLAGS += -I$(ETHERNET_LIBRARY_PATH)
INCLUDE_FLAGS += -I$(ETHERNET_UTIL_LIBRARY_PATH)
INCLUDE_FLAGS += -I$(SPI_LIBRARY_PATH)
#----------------------------------------------------------

# generate lists of output files (in build dir)
SPI_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(SPI_OBJS))
ARDUINO_CORE_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(ARDUINO_CORE_OBJS))
ARDUINO_CORE_C_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(ARDUINO_CORE_C_OBJS))
ETHERNET_UTIL_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(ETHERNET_UTIL_OBJS))
ETHERNET_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(ETHERNET_OBJS))
PROJECT_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(PROJECT_OBJS))

#--------------------------------------------------------------
# rules:
all: build size
build: elf hex

# upload the program to the microcontroller flash:
upload:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)

# analyze compiled program size:
size:
	$(SIZE) --target=$(FORMAT) $(TARGET).hex

elf: $(TARGET).elf
hex: $(TARGET).hex

# link everything together:
$(TARGET).elf: core ethernet spi project
	$(CPP) $(CPP_FLAGS) $(SPI_OBJS_FIN) $(ARDUINO_CORE_C_OBJS_FIN) $(ARDUINO_CORE_OBJS_FIN) $(ETHERNET_OBJS_FIN) $(ETHERNET_UTIL_OBJS_FIN) $(PROJECT_OBJS_FIN) -o $(TARGET).elf $(LDFLAGS)

# convert elf to hex file for programmer:
# exclude eeprom section, in case it exists
# as we do not want to write this to flash
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

clean:
	rm -rv $(BUILD_DIR)/*
	rm -v $(TARGET).elf
	rm -v $(TARGET).hex
	rm -v $(TARGET).map

ethernet: $(ETHERNET_OBJS_FIN) $(ETHERNET_UTIL_OBJS_FIN)

spi: $(SPI_OBJS_FIN)

core: $(ARDUINO_CORE_OBJS_FIN) $(ARDUINO_CORE_C_OBJS_FIN)

project: $(PROJECT_OBJS_FIN)

# build arduino core
$(ARDUINO_CORE_OBJS_FIN): %.o: 
	$(CPP) -c $(CPP_FLAGS) $(ARDUINO_CORE_PATH)/$(subst $(BUILD_DIR)/,,$*).cpp -o $*.o
$(ARDUINO_CORE_C_OBJS_FIN): %.o:
	$(CC) -c $(C_FLAGS) $(ARDUINO_CORE_PATH)/$(subst $(BUILD_DIR)/,,$*).c -o $*.o

# build SPI
$(SPI_OBJS_FIN): %.o:
	$(CPP) -c $(CPP_FLAGS) $(SPI_LIBRARY_PATH)/$(subst $(BUILD_DIR)/,,$*).cpp -o $*.o

# build Ethernet library
$(ETHERNET_OBJS_FIN): %.o:
	$(CPP) -c $(CPP_FLAGS) $(ETHERNET_LIBRARY_PATH)/$(subst $(BUILD_DIR)/,,$*).cpp -o $*.o
$(ETHERNET_UTIL_OBJS_FIN): %.o:
	$(CPP) -c $(CPP_FLAGS) $(ETHERNET_UTIL_LIBRARY_PATH)/$(subst $(BUILD_DIR)/,,$*).cpp -o $*.o

# build Project files
$(PROJECT_OBJS_FIN): %.o: $(patsubst %.o,%.cpp,$(PROJECT_OBJS))
	$(CPP) -c $(CPP_FLAGS) $(PROJECT_PATH)/$(subst $(BUILD_DIR)/,,$*).cpp -o $*.o
