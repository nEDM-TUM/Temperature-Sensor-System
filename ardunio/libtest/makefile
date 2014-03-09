MCU = atmega328
F_CPU = 8000000UL

ARDUINO_PATH = /home/wenwen/uni/s56/idp/Arduino-master

BOARD_TYPE = ethernet

BUILD_DIR = ./

TARGET = main

FORMAT = ihex

# Project files
PROJECT_PATH = ./
PROJECT_OBJS = ChatServer.o
#----------------------------------------------------------

LDFLAGS = -Wl,-Map=$(TARGET).map,--cref
# -Wl and --gc-sections are needed to avoid __cxa_pure_virtual() reference errors
#  which are caused by arduino c++ virtual functions
LDFLAGS += -Wl,--gc-sections

# this specifies which part of the Arduino Core library to compile:
# Arduino core library consists of C++ AND C sources
# => handle them separately here
ARDUINO_CORE_PATH = $(ARDUINO_PATH)/hardware/arduino/cores/arduino
ARDUINO_CORE_OBJS = IPAddress.o Print.o WString.o WMath.o 
ARDUINO_CORE_C_OBJS = wiring_digital.o wiring.o

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

# -mmcu selects the AVR chip type -> specifies instructionset etc.
# -DF_CPU defines the F_CPU preprocessor makro
CPP_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(INCLUDE_FLAGS) -Os
# -ffunction-sections -fdata-sections are needed to avoid
#  __cxa_pure_virtual() reference errors which are caused by
#  arduino c++ virtual functions
CPP_FLAGS += -ffunction-sections -fdata-sections
C_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(INCLUDE_FLAGS) -Os

# generate lists of output files (in build dir)
SPI_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(SPI_OBJS))
ARDUINO_CORE_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(ARDUINO_CORE_OBJS))
ARDUINO_CORE_C_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(ARDUINO_CORE_C_OBJS))
ETHERNET_UTIL_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(ETHERNET_UTIL_OBJS))
ETHERNET_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(ETHERNET_OBJS))
PROJECT_OBJS_FIN=$(addprefix $(BUILD_DIR)/,$(PROJECT_OBJS))

# compiler suite
CC=avr-gcc
CPP=avr-g++
# AR=avr-ar # we currently do not package the libraries in .a files, so this is not needed
OBJCOPY = avr-objcopy
SIZE = avr-size
AVRDUDE = avrdude

#--------------------------------------------------------------
# rules:
all: build size
build: elf hex

# upload the program to the microcontroller flash:
upload:
	$(AVRDUDE) 

# analyze compiled program size:
size:
	$(SIZE) --target=$(FORMAT) $(TARGET).hex

elf: $(TARGET).elf
hex: $(TARGET).hex

$(TARGET).elf: core ethernet spi project
	$(CPP) $(CPP_FLAGS) $(SPI_OBJS_FIN) $(ARDUINO_CORE_C_OBJS_FIN) $(ARDUINO_CORE_OBJS_FIN) $(ETHERNET_OBJS_FIN) $(ETHERNET_UTIL_OBJS_FIN) $(PROJECT_OBJS_FIN) -o $(TARGET).elf $(LDFLAGS)

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

clean:
	rm -rv $(BUILD_DIR)/*
	rm -v $(TARGET).elf
	rm -v $(TARGET).hex
	rm -v $(TARGET).map

ethernet: $(ETHERNET_OBJS) $(ETHERNET_UTIL_OBJS)

spi: $(SPI_OBJS)

core: $(ARDUINO_CORE_OBJS) $(ARDUINO_CORE_C_OBJS)

project: $(PROJECT_OBJS)

$(ARDUINO_CORE_OBJS): %.o: 
	$(CPP) -c $(CPP_FLAGS) $(ARDUINO_CORE_PATH)/$*.cpp -o $(BUILD_DIR)/$*.o

$(ARDUINO_CORE_C_OBJS): %.o:
	$(CC) -c $(C_FLAGS) $(ARDUINO_CORE_PATH)/$*.c -o $(BUILD_DIR)/$*.o

$(SPI_OBJS): %.o:
	$(CPP) $(SPI_LIBRARY_PATH)/$*.cpp -c $(CPP_FLAGS) -o $(BUILD_DIR)/$*.o

$(ETHERNET_OBJS): %.o:
	$(CPP) $(ETHERNET_LIBRARY_PATH)/$*.cpp -c $(CPP_FLAGS) -o $(BUILD_DIR)/$*.o
$(ETHERNET_UTIL_OBJS): %.o:
	$(CPP) $(ETHERNET_UTIL_LIBRARY_PATH)/$*.cpp -c $(CPP_FLAGS) -o $(BUILD_DIR)/$*.o

$(PROJECT_OBJS): %.o:
	$(CPP) -c $(CPP_FLAGS) $(PROJECT_PATH)/$*.cpp -o $(BUILD_DIR)/$*.o
