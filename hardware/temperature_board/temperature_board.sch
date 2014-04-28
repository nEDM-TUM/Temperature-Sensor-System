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
LIBS:temperature_board-cache
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date "28 apr 2014"
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
L TSIC50XF U?
U 1 1 532DDB20
P 4000 3200
F 0 "U?" H 4200 3650 70  0000 C CNN
F 1 "TSIC50XF" H 4000 3000 70  0000 C CNN
F 2 "~" H 4000 3200 60  0000 C CNN
F 3 "~" H 4000 3200 60  0000 C CNN
	1    4000 3200
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
Wire Wire Line
	7700 2750 7800 2750
Wire Wire Line
	7700 3050 7800 3050
Wire Wire Line
	7800 3050 7800 3650
Wire Wire Line
	7800 2750 7800 2550
Wire Wire Line
	7800 2550 8900 2550
Wire Wire Line
	8000 2550 8000 2650
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
Wire Wire Line
	6800 1550 6800 1850
Wire Wire Line
	6800 3850 6800 4300
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
Wire Wire Line
	6800 1700 7050 1700
Connection ~ 6800 1700
Wire Wire Line
	7450 1700 7650 1700
Wire Wire Line
	7650 1700 7650 1900
$Comp
L GND #PWR?
U 1 1 532DDC5E
P 4000 4500
F 0 "#PWR?" H 4000 4500 30  0001 C CNN
F 1 "GND" H 4000 4430 30  0001 C CNN
F 2 "" H 4000 4500 60  0000 C CNN
F 3 "" H 4000 4500 60  0000 C CNN
	1    4000 4500
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 532DDC6A
P 4650 2150
F 0 "C?" H 4650 2250 40  0000 L CNN
F 1 "100nF" H 4656 2065 40  0000 L CNN
F 2 "~" H 4688 2000 30  0000 C CNN
F 3 "~" H 4650 2150 60  0000 C CNN
	1    4650 2150
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4000 4500 4000 4050
Wire Wire Line
	4450 2150 4000 2150
Wire Wire Line
	4000 1950 4000 2350
$Comp
L R R?
U 1 1 532DDC9D
P 4000 1700
F 0 "R?" V 4080 1700 40  0000 C CNN
F 1 "220" V 4007 1701 40  0000 C CNN
F 2 "~" V 3930 1700 30  0000 C CNN
F 3 "~" H 4000 1700 30  0000 C CNN
	1    4000 1700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 532DDCAA
P 5000 2550
F 0 "#PWR?" H 5000 2550 30  0001 C CNN
F 1 "GND" H 5000 2480 30  0001 C CNN
F 2 "" H 5000 2550 60  0000 C CNN
F 3 "" H 5000 2550 60  0000 C CNN
	1    5000 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 2150 5000 2150
Wire Wire Line
	5000 2150 5000 2550
Connection ~ 4000 2150
$Comp
L VCC #PWR?
U 1 1 532DDCEC
P 4000 1250
F 0 "#PWR?" H 4000 1350 30  0001 C CNN
F 1 "VCC" H 4000 1350 30  0000 C CNN
F 2 "" H 4000 1250 60  0000 C CNN
F 3 "" H 4000 1250 60  0000 C CNN
	1    4000 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 1250 4000 1450
Wire Wire Line
	4650 3200 5900 3200
Wire Wire Line
	5500 2000 5500 3000
Wire Wire Line
	5500 2800 5900 2800
Wire Wire Line
	5500 3000 5900 3000
Connection ~ 5500 2800
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
Wire Wire Line
	8900 3250 8800 3250
Wire Wire Line
	8800 3250 8800 3500
Wire Wire Line
	8700 3150 8900 3150
$Comp
L VCC #PWR?
U 1 1 532DE001
P 8700 3050
F 0 "#PWR?" H 8700 3150 30  0001 C CNN
F 1 "VCC" H 8700 3150 30  0000 C CNN
F 2 "" H 8700 3050 60  0000 C CNN
F 3 "" H 8700 3050 60  0000 C CNN
	1    8700 3050
	1    0    0    -1  
$EndComp
Connection ~ 8000 2550
$Comp
L C C?
U 1 1 534839D5
P 8000 3450
F 0 "C?" H 8000 3550 40  0000 L CNN
F 1 "100nF" H 8006 3365 40  0000 L CNN
F 2 "~" H 8038 3300 30  0000 C CNN
F 3 "~" H 8000 3450 60  0000 C CNN
	1    8000 3450
	-1   0    0    1   
$EndComp
Wire Wire Line
	8000 3150 8000 3250
Wire Wire Line
	7800 3650 8200 3650
Wire Wire Line
	8200 3650 8200 2650
Wire Wire Line
	8200 2650 8900 2650
Connection ~ 8000 3650
$Comp
L R R?
U 1 1 53483A7B
P 8300 1950
F 0 "R?" V 8380 1950 40  0000 C CNN
F 1 "22k" V 8307 1951 40  0000 C CNN
F 2 "~" V 8230 1950 30  0000 C CNN
F 3 "~" H 8300 1950 30  0000 C CNN
	1    8300 1950
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 53483A81
P 8450 3200
F 0 "R?" V 8530 3200 40  0000 C CNN
F 1 "22k" V 8457 3201 40  0000 C CNN
F 2 "~" V 8380 3200 30  0000 C CNN
F 3 "~" H 8450 3200 30  0000 C CNN
	1    8450 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	8300 2200 8300 2650
Connection ~ 8300 2650
Wire Wire Line
	8450 2950 8450 2550
Connection ~ 8450 2550
$Comp
L GND #PWR?
U 1 1 53483B09
P 8450 3750
F 0 "#PWR?" H 8450 3750 30  0001 C CNN
F 1 "GND" H 8450 3680 30  0001 C CNN
F 2 "" H 8450 3750 60  0000 C CNN
F 3 "" H 8450 3750 60  0000 C CNN
	1    8450 3750
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 53483B0F
P 8300 1550
F 0 "#PWR?" H 8300 1650 30  0001 C CNN
F 1 "VCC" H 8300 1650 30  0000 C CNN
F 2 "" H 8300 1550 60  0000 C CNN
F 3 "" H 8300 1550 60  0000 C CNN
	1    8300 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	8300 1550 8300 1700
Wire Wire Line
	8450 3450 8450 3750
Wire Wire Line
	8700 3050 8700 3150
$Comp
L GND #PWR?
U 1 1 534937C6
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
Text Notes 9150 2300 0    60   ~ 0
SWAP A AND B\nA=1\nB=2
$EndSCHEMATC
