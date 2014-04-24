EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:TSic50xF
LIBS:ST485
LIBS:humidity_board-cache
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date "12 apr 2014"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L VCC #PWR?
U 1 1 532DBFEE
P 5500 2000
F 0 "#PWR?" H 5500 2100 30  0001 C CNN
F 1 "VCC" H 5500 2100 30  0000 C CNN
F 2 "" H 5500 2000 60  0000 C CNN
F 3 "" H 5500 2000 60  0000 C CNN
	1    5500 2000
	1    0    0    -1  
$EndComp
$Comp
L ST485 U?
U 1 1 532DDB2F
P 6800 2900
F 0 "U?" H 7250 3500 70  0000 C BNN
F 1 "ST485" H 7200 2350 70  0000 C CNN
F 2 "" H 6800 2800 60  0000 C CNN
F 3 "" H 6800 2800 60  0000 C CNN
	1    6800 2900
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 532DDB70
P 8000 2900
F 0 "R?" V 8080 2900 40  0000 C CNN
F 1 "120" V 8007 2901 40  0000 C CNN
F 2 "~" V 7930 2900 30  0000 C CNN
F 3 "~" H 8000 2900 30  0000 C CNN
	1    8000 2900
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 532DDBFC
P 6800 4300
F 0 "#PWR?" H 6800 4300 30  0001 C CNN
F 1 "GND" H 6800 4230 30  0001 C CNN
F 2 "" H 6800 4300 60  0000 C CNN
F 3 "" H 6800 4300 60  0000 C CNN
	1    6800 4300
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 532DDC02
P 6800 1550
F 0 "#PWR?" H 6800 1650 30  0001 C CNN
F 1 "VCC" H 6800 1650 30  0000 C CNN
F 2 "" H 6800 1550 60  0000 C CNN
F 3 "" H 6800 1550 60  0000 C CNN
	1    6800 1550
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 532DDC2C
P 7250 1700
F 0 "C?" H 7250 1800 40  0000 L CNN
F 1 "100nF" H 7256 1615 40  0000 L CNN
F 2 "~" H 7288 1550 30  0000 C CNN
F 3 "~" H 7250 1700 60  0000 C CNN
	1    7250 1700
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR?
U 1 1 532DDC32
P 7650 1900
F 0 "#PWR?" H 7650 1900 30  0001 C CNN
F 1 "GND" H 7650 1830 30  0001 C CNN
F 2 "" H 7650 1900 60  0000 C CNN
F 3 "" H 7650 1900 60  0000 C CNN
	1    7650 1900
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 532DDC5E
P 6050 7150
F 0 "#PWR?" H 6050 7150 30  0001 C CNN
F 1 "GND" H 6050 7080 30  0001 C CNN
F 2 "" H 6050 7150 60  0000 C CNN
F 3 "" H 6050 7150 60  0000 C CNN
	1    6050 7150
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 532DDC6A
P 6450 5100
F 0 "C?" H 6450 5200 40  0000 L CNN
F 1 "100nF" H 6456 5015 40  0000 L CNN
F 2 "~" H 6488 4950 30  0000 C CNN
F 3 "~" H 6450 5100 60  0000 C CNN
	1    6450 5100
	0    -1   -1   0   
$EndComp
$Comp
L R R?
U 1 1 532DDC9D
P 6050 4650
F 0 "R?" V 6130 4650 40  0000 C CNN
F 1 "220" V 6057 4651 40  0000 C CNN
F 2 "~" V 5980 4650 30  0000 C CNN
F 3 "~" H 6050 4650 30  0000 C CNN
	1    6050 4650
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 532DDCAA
P 6950 5500
F 0 "#PWR?" H 6950 5500 30  0001 C CNN
F 1 "GND" H 6950 5430 30  0001 C CNN
F 2 "" H 6950 5500 60  0000 C CNN
F 3 "" H 6950 5500 60  0000 C CNN
	1    6950 5500
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 532DDCEC
P 6050 4300
F 0 "#PWR?" H 6050 4400 30  0001 C CNN
F 1 "VCC" H 6050 4400 30  0000 C CNN
F 2 "" H 6050 4300 60  0000 C CNN
F 3 "" H 6050 4300 60  0000 C CNN
	1    6050 4300
	1    0    0    -1  
$EndComp
$Comp
L RJ45 J?
U 1 1 532DDDA3
P 9350 2900
F 0 "J?" H 9550 3400 60  0000 C CNN
F 1 "RJ45" H 9200 3400 60  0000 C CNN
F 2 "~" H 9350 2900 60  0000 C CNN
F 3 "~" H 9350 2900 60  0000 C CNN
	1    9350 2900
	0    1    1    0   
$EndComp
$Comp
L GND #PWR?
U 1 1 532DDE77
P 8800 3500
F 0 "#PWR?" H 8800 3500 30  0001 C CNN
F 1 "GND" H 8800 3430 30  0001 C CNN
F 2 "" H 8800 3500 60  0000 C CNN
F 3 "" H 8800 3500 60  0000 C CNN
	1    8800 3500
	1    0    0    -1  
$EndComp
$Comp
L HYT271 U?
U 1 1 532DEF53
P 6050 6050
F 0 "U?" H 6250 6500 70  0000 C CNN
F 1 "HYT271" H 6050 5800 70  0000 C CNN
F 2 "~" H 6050 6050 60  0000 C CNN
F 3 "~" H 6050 6050 60  0000 C CNN
	1    6050 6050
	1    0    0    -1  
$EndComp
$Comp
L ATMEGA88-A IC?
U 1 1 533024BD
P 2500 2550
F 0 "IC?" H 1750 3800 40  0000 L BNN
F 1 "ATMEGA88-A" H 2950 1150 40  0000 L BNN
F 2 "TQFP32" H 2500 2550 30  0000 C CIN
F 3 "~" H 2500 2550 60  0000 C CNN
	1    2500 2550
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5330251C
P 1400 4050
F 0 "#PWR?" H 1400 4050 30  0001 C CNN
F 1 "GND" H 1400 3980 30  0001 C CNN
F 2 "" H 1400 4050 60  0000 C CNN
F 3 "" H 1400 4050 60  0000 C CNN
	1    1400 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	7700 2750 7800 2750
Wire Wire Line
	7700 3050 7800 3050
Wire Wire Line
	7800 3050 7800 3800
Wire Wire Line
	7800 2750 7800 2550
Wire Wire Line
	7800 2550 8900 2550
Wire Wire Line
	8000 2550 8000 2650
Wire Wire Line
	6800 1550 6800 1850
Wire Wire Line
	6800 3850 6800 4300
Wire Wire Line
	6800 1700 7050 1700
Connection ~ 6800 1700
Wire Wire Line
	7450 1700 7650 1700
Wire Wire Line
	7650 1700 7650 1900
Wire Wire Line
	6250 5100 6050 5100
Wire Wire Line
	6650 5100 6950 5100
Wire Wire Line
	6950 5100 6950 5500
Connection ~ 6050 5100
Wire Wire Line
	6050 4300 6050 4400
Wire Wire Line
	5500 2000 5500 3000
Wire Wire Line
	5500 2800 5900 2800
Wire Wire Line
	5500 3000 5900 3000
Connection ~ 5500 2800
Wire Wire Line
	8900 3250 8800 3250
Wire Wire Line
	8800 3250 8800 3500
Wire Wire Line
	8700 3150 8900 3150
Connection ~ 8000 2550
Wire Wire Line
	8900 2650 8200 2650
Wire Wire Line
	8200 2650 8200 3800
Wire Wire Line
	6050 4900 6050 5200
Wire Wire Line
	6050 7150 6050 6900
Wire Wire Line
	1600 3550 1400 3550
Wire Wire Line
	1400 3550 1400 4050
Wire Wire Line
	1600 3650 1400 3650
Connection ~ 1400 3650
Wire Wire Line
	1600 3750 1400 3750
Connection ~ 1400 3750
$Comp
L VCC #PWR?
U 1 1 53302620
P 1450 1000
F 0 "#PWR?" H 1450 1100 30  0001 C CNN
F 1 "VCC" H 1450 1100 30  0000 C CNN
F 2 "" H 1450 1000 60  0000 C CNN
F 3 "" H 1450 1000 60  0000 C CNN
	1    1450 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	1450 1000 1450 1750
Wire Wire Line
	1450 1450 1600 1450
Wire Wire Line
	1450 1550 1600 1550
Connection ~ 1450 1450
Wire Wire Line
	1450 1750 1600 1750
Connection ~ 1450 1550
Text Label 3800 2700 0    60   ~ 0
SDA
Text Label 7300 5950 0    60   ~ 0
SDA
Text Label 7300 6150 0    60   ~ 0
SCL
Text Label 3800 2800 0    60   ~ 0
SCL
Wire Wire Line
	3800 2700 3500 2700
Wire Wire Line
	3800 2800 3500 2800
Wire Wire Line
	7300 5950 6700 5950
Wire Wire Line
	7300 6150 6700 6150
Text Label 5300 3200 0    60   ~ 0
DI
Text Label 3800 3650 0    60   ~ 0
DI
Wire Wire Line
	5300 3200 5900 3200
Wire Wire Line
	3800 3650 3500 3650
$Comp
L LED D?
U 1 1 53302A6A
P 1300 6700
F 0 "D?" H 1300 6800 50  0000 C CNN
F 1 "LED" H 1300 6600 50  0000 C CNN
F 2 "~" H 1300 6700 60  0000 C CNN
F 3 "~" H 1300 6700 60  0000 C CNN
	1    1300 6700
	-1   0    0    1   
$EndComp
$Comp
L CONN_5X2 P?
U 1 1 53302A79
P 1800 5550
F 0 "P?" H 1800 5850 60  0000 C CNN
F 1 "CONN_5X2" V 1800 5550 50  0000 C CNN
F 2 "~" H 1800 5550 60  0000 C CNN
F 3 "~" H 1800 5550 60  0000 C CNN
	1    1800 5550
	1    0    0    -1  
$EndComp
Text Label 3800 1750 0    60   ~ 0
MOSI
Text Label 3800 1850 0    60   ~ 0
MISO
Text Label 3800 1950 0    60   ~ 0
SCK
Text Label 3800 2900 0    60   ~ 0
RESET
Text Label 1050 5550 0    60   ~ 0
RESET
Text Label 1050 5650 0    60   ~ 0
SCK
Text Label 1050 5350 0    60   ~ 0
MOSI
Text Label 1050 5750 0    60   ~ 0
MISO
$Comp
L GND #PWR?
U 1 1 53302A98
P 2400 6000
F 0 "#PWR?" H 2400 6000 30  0001 C CNN
F 1 "GND" H 2400 5930 30  0001 C CNN
F 2 "" H 2400 6000 60  0000 C CNN
F 3 "" H 2400 6000 60  0000 C CNN
	1    2400 6000
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 53302A9E
P 2350 5100
F 0 "#PWR?" H 2350 5200 30  0001 C CNN
F 1 "VCC" H 2350 5200 30  0000 C CNN
F 2 "" H 2350 5100 60  0000 C CNN
F 3 "" H 2350 5100 60  0000 C CNN
	1    2350 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2350 5100 2350 5350
Wire Wire Line
	2350 5350 2200 5350
Wire Wire Line
	2400 5550 2400 6000
Wire Wire Line
	2400 5550 2200 5550
Wire Wire Line
	2200 5650 2400 5650
Connection ~ 2400 5650
Wire Wire Line
	2200 5750 2400 5750
Connection ~ 2400 5750
Wire Wire Line
	1050 5350 1400 5350
Wire Wire Line
	1050 5550 1400 5550
Wire Wire Line
	1050 5650 1400 5650
Wire Wire Line
	1050 5750 1400 5750
Wire Wire Line
	3800 2900 3500 2900
Wire Wire Line
	3800 1950 3500 1950
Wire Wire Line
	3800 1850 3500 1850
Wire Wire Line
	3800 1750 3500 1750
Text Label 3800 2300 0    60   ~ 0
LED1
Text Label 3800 2400 0    60   ~ 0
LED2
Text Label 700  6700 0    60   ~ 0
LED1
$Comp
L LED D?
U 1 1 53302E6F
P 1300 7150
F 0 "D?" H 1300 7250 50  0000 C CNN
F 1 "LED" H 1300 7050 50  0000 C CNN
F 2 "~" H 1300 7150 60  0000 C CNN
F 3 "~" H 1300 7150 60  0000 C CNN
	1    1300 7150
	-1   0    0    1   
$EndComp
Text Label 700  7150 0    60   ~ 0
LED2
Wire Wire Line
	1100 6700 700  6700
Wire Wire Line
	700  7150 1100 7150
Wire Wire Line
	3800 2300 3500 2300
Wire Wire Line
	3800 2400 3500 2400
$Comp
L SW_PUSH SW?
U 1 1 533030C7
P 3950 7100
F 0 "SW?" H 4100 7210 50  0000 C CNN
F 1 "SW_PUSH" H 3950 7020 50  0000 C CNN
F 2 "~" H 3950 7100 60  0000 C CNN
F 3 "~" H 3950 7100 60  0000 C CNN
	1    3950 7100
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 533030D4
P 4500 7450
F 0 "#PWR?" H 4500 7450 30  0001 C CNN
F 1 "GND" H 4500 7380 30  0001 C CNN
F 2 "" H 4500 7450 60  0000 C CNN
F 3 "" H 4500 7450 60  0000 C CNN
	1    4500 7450
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 533030DC
P 3500 6550
F 0 "R?" V 3580 6550 40  0000 C CNN
F 1 "10k" V 3507 6551 40  0000 C CNN
F 2 "~" V 3430 6550 30  0000 C CNN
F 3 "~" H 3500 6550 30  0000 C CNN
	1    3500 6550
	1    0    0    -1  
$EndComp
Text Label 3000 7100 0    60   ~ 0
RESET
$Comp
L VCC #PWR?
U 1 1 533030EB
P 3500 5950
F 0 "#PWR?" H 3500 6050 30  0001 C CNN
F 1 "VCC" H 3500 6050 30  0000 C CNN
F 2 "" H 3500 5950 60  0000 C CNN
F 3 "" H 3500 5950 60  0000 C CNN
	1    3500 5950
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 5950 3500 6300
Wire Wire Line
	3000 7100 3650 7100
Wire Wire Line
	4250 7100 4500 7100
Wire Wire Line
	4500 7100 4500 7450
Wire Wire Line
	3500 6800 3500 7100
Connection ~ 3500 7100
$Comp
L VCC #PWR?
U 1 1 5330344D
P 2400 6400
F 0 "#PWR?" H 2400 6500 30  0001 C CNN
F 1 "VCC" H 2400 6500 30  0000 C CNN
F 2 "" H 2400 6400 60  0000 C CNN
F 3 "" H 2400 6400 60  0000 C CNN
	1    2400 6400
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 53303458
P 2000 6700
F 0 "R?" V 2080 6700 40  0000 C CNN
F 1 "150" V 2007 6701 40  0000 C CNN
F 2 "~" V 1930 6700 30  0000 C CNN
F 3 "~" H 2000 6700 30  0000 C CNN
	1    2000 6700
	0    -1   -1   0   
$EndComp
$Comp
L R R?
U 1 1 5330345E
P 2000 7150
F 0 "R?" V 2080 7150 40  0000 C CNN
F 1 "150" V 2007 7151 40  0000 C CNN
F 2 "~" V 1930 7150 30  0000 C CNN
F 3 "~" H 2000 7150 30  0000 C CNN
	1    2000 7150
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2400 6400 2400 6700
Wire Wire Line
	2400 6700 2250 6700
Wire Wire Line
	1750 6700 1500 6700
$Comp
L VCC #PWR?
U 1 1 53303543
P 2400 6900
F 0 "#PWR?" H 2400 7000 30  0001 C CNN
F 1 "VCC" H 2400 7000 30  0000 C CNN
F 2 "" H 2400 6900 60  0000 C CNN
F 3 "" H 2400 6900 60  0000 C CNN
	1    2400 6900
	1    0    0    -1  
$EndComp
Wire Wire Line
	2400 6900 2400 7150
Wire Wire Line
	2400 7150 2250 7150
Wire Wire Line
	1750 7150 1500 7150
$Comp
L VCC #PWR?
U 1 1 53493890
P 8700 2950
F 0 "#PWR?" H 8700 3050 30  0001 C CNN
F 1 "VCC" H 8700 3050 30  0000 C CNN
F 2 "" H 8700 2950 60  0000 C CNN
F 3 "" H 8700 2950 60  0000 C CNN
	1    8700 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	8700 2950 8700 3150
$Comp
L C C?
U 1 1 534938E9
P 8000 3500
F 0 "C?" H 8000 3600 40  0000 L CNN
F 1 "100nF" H 8006 3415 40  0000 L CNN
F 2 "~" H 8038 3350 30  0000 C CNN
F 3 "~" H 8000 3500 60  0000 C CNN
	1    8000 3500
	-1   0    0    1   
$EndComp
Wire Wire Line
	8200 3800 7800 3800
Wire Wire Line
	8000 3700 8000 3800
Connection ~ 8000 3800
Wire Wire Line
	8000 3150 8000 3300
$Comp
L R R?
U 1 1 534939E7
P 8350 2000
F 0 "R?" V 8430 2000 40  0000 C CNN
F 1 "22k" V 8357 2001 40  0000 C CNN
F 2 "~" V 8280 2000 30  0000 C CNN
F 3 "~" H 8350 2000 30  0000 C CNN
	1    8350 2000
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 534939ED
P 8500 3050
F 0 "R?" V 8580 3050 40  0000 C CNN
F 1 "22k" V 8507 3051 40  0000 C CNN
F 2 "~" V 8430 3050 30  0000 C CNN
F 3 "~" H 8500 3050 30  0000 C CNN
	1    8500 3050
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 534939F3
P 8500 3500
F 0 "#PWR?" H 8500 3500 30  0001 C CNN
F 1 "GND" H 8500 3430 30  0001 C CNN
F 2 "" H 8500 3500 60  0000 C CNN
F 3 "" H 8500 3500 60  0000 C CNN
	1    8500 3500
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 534939F9
P 8350 1600
F 0 "#PWR?" H 8350 1700 30  0001 C CNN
F 1 "VCC" H 8350 1700 30  0000 C CNN
F 2 "" H 8350 1600 60  0000 C CNN
F 3 "" H 8350 1600 60  0000 C CNN
	1    8350 1600
	1    0    0    -1  
$EndComp
Wire Wire Line
	8350 1600 8350 1750
Wire Wire Line
	8350 2250 8350 2650
Connection ~ 8350 2650
Wire Wire Line
	8500 2800 8500 2550
Connection ~ 8500 2550
Wire Wire Line
	8500 3300 8500 3500
$Comp
L GND #PWR?
U 1 1 53493BBC
P 9700 3600
F 0 "#PWR?" H 9700 3600 30  0001 C CNN
F 1 "GND" H 9700 3530 30  0001 C CNN
F 2 "" H 9700 3600 60  0000 C CNN
F 3 "" H 9700 3600 60  0000 C CNN
	1    9700 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	9700 3450 9700 3600
$EndSCHEMATC
