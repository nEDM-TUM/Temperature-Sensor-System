Temperature-Sensor-System
==============

Sensor network system built for the nEDM experiment.

Features
------------------
- Both temperature and humidity sensors can be connected at any sensor socket.
The sensor type will be automatically detected.
- High temperature and humidity accuracy: Temperature sensors are calibrated to ±0.1K and the humidity sensors to ±1.8%rH.
- A high number of connected sensors is supported. Currently 44 sensors are in use, however the system is designed to support at least 100 sensors and still be able to fulfill a sampling rate of 1Hz. If the sampling rate is allowed to be slower, up to 960 sensors are possible to share one Ethernet connection.
- The system supports a fast sample rate of at least 1Hz.
- We achieved a relatively low cost per sensor. The cost per sensor module is mostly defined by the sensor chip alone, which is about e7 for a precision temperature sensor and about e20 for a high quality humidity sensor. The cost for the printed circuit board and required components is less than e4 per board.
- Measurement results can be forwarded via TCP/IP to a CouchDB server, which is able to record the measurement data from all connected sensors.
- The sensor network provides a TCP/telnet user interface, for controlling and viewing of network parameters. This allows easy configuring and debugging of the system.
- Most of the system parameters like sampling rate or database IP addresses and insert functions are configurable at runtime.
- Plug and Play is supported for adding and removing sensors or even control units during runtime.

Directory structure
--------------------
```
.
+-- casings     // 3D CAD files for 3D printer
|   +-- adapter
|   +-- arduino
|   +-- cable_clamp
|   +-- collector
|   +-- humidity
|   +-- temperature
+-- circuit_boards      // Circuit schematic diagram and 
|   |                   // PCB schematic files created using KiCad
|   +-- 3x127_254_adapter       // A PCB chip adapter, 
|   |                           // used for breadboard debugging
|   +-- 4x127_254_adapter       // A PCB chip adapter, 
|   |                           // used for breadboard debugging
|   +-- arduino_adapter
|   +-- humidity
|   +-- kicad_library           // Footprint and Component
|   |                           // library files for KiCad
|   +-- order           // PCB assembly, which was sent to 
|   |                   // Fischer Leiterplatten for production
|   +-- sensor_board
|   +-- so8_breakout    // A PCB chip adapter, 
|   |                   // used for breadboard debugging
|   +-- temperature_board
+-- datasheets      // Datasheets for sensors, micro processor, etc.
+-- presentation    // Slides (tex files) for a project presentation
+-- registration    
|   +-- description     // Project description (tex files) 
|                       // used for registration
+-- report      // Main project documentation (tex files)
+-- shop_orders // Receipts of orders from hardware shops
+-- sofware
|   +-- avr_wcet_analyzer   // code of a WCET analyzer
|   |                       // for the AVR instruction set
|   +-- collector_board      
|   +-- ethernet_board        
|   +-- humidity_board      
|   +-- util        // Commonly used utility code
+-- squid_analysis      
|   +-- data            // Squid magnetic field measurement protocol.
|                       // For squid data please write a mail to us.
|   +-- evaluation      // Evaluation script of squid analysis data
```

Tips for developers
--------------------
- To prevent EEPROM from being corrupted on power cycles, the AVR integrated brownout detection should be enabled.
- To allow EEPROM storage to stay intact after reprogramming, set the corresponding fuse bit.
- As Arduino fuses are not possible to be programmed via the default Arduino programmer (using the Arduino boot loader), we removed the boot loader and use the ISP connector for programming.
- To not overload the voltage regulator, especially when driving multiple boards one should consider, that a significant portion of power is used to drive LEDs. So turning the LEDs off during normal operation will prevent overheating.
- When debugging and programming devices of the sensor network, one has to be very careful, to not provide different power sources, as this might cause unexpected results, which can lead to damage of the sensor network, as well as connected computers. By default the USBasp programmer provides the system with 5V power.
- Our makefiles currently do not support automatic detection of file dependencies, and source files to be compiled. All source files have to be specified explicitly and if changes to header files are made, a ’make clean’ should be performed before compiling the program.
- Be careful, when using the USART line or similar for generating debugging output. Transmitting debugging information takes relatively long time and might influence the system heavily, especially, when debugging timing critical code. LEDs or GPIO pins with an oscilloscope provide a faster but more complex way for debugging.

###For more details about our project, please read the report document. ###
Its tex file and needed images can be found in the folder _report_.
