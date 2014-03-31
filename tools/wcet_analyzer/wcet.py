#!/usr/bin/python2

# usage:  avr-objdump main.elf | ./wcet.py [section_search_pattern]

import sys
import re


search_section = ''
found = False
if (len(sys.argv) == 2):
    print "searching: " + str(sys.argv[1])
    search_section = str(sys.argv[1])


optime = {}
f = open('opcodes.lst', 'r')
for line in f:
    m = re.search('^([a-z]*)\s+([0-9])+\s+.*$',line)
    if m:
        #print 'instr ' + m.group(1) + " time " + m.group(2)
        optime[m.group(1)] = int(m.group(2))
#print optime

total_used_instr = {}
total_cycles = 0
section_used_instr = {}
section_cycles = 0
for line in sys.stdin:
    m = re.search('^\s*([0-9a-f]+):(?:\s*[0-9a-f])*\s+([a-z]+)\s+(.*)$', line)
    #m = re.search('^\s.*$', line)
    if m:
        #print m.group(0)
        instr = m.group(2)
        current_optime = optime[instr]
        if (found == True or search_section == ''):
            print str(section_cycles) + " + " + str(current_optime) + " " + m.group(1) + ": " + m.group(2) + " \t" + m.group(3)
        total_cycles = total_cycles + current_optime
        section_cycles = section_cycles + current_optime
        try:
            (count, cycles) = total_used_instr[instr]
            total_used_instr[instr] = (count + 1 , cycles + current_optime)
        except KeyError:
            total_used_instr[instr] = (1, current_optime)
        try:
            (count, cycles) = total_used_instr[instr]
            section_used_instr[instr] = (count + 1 , cycles + current_optime)
        except KeyError:
            section_used_instr[instr] = (1, current_optime)
    else:
        m = re.search('^\s*([0-9a-f]+)\s+\<(.*)\>:$', line)
        if m:
            if ( total_cycles != 0 ):
                if (found == True or search_section == ''):
                    print ""
                    print "Section summary:"
                    print "Maximum cycles: " + str(section_cycles)
                    print "Used instructions:"
                    for i in section_used_instr:
                        (count2, cycles2) = section_used_instr[i]
                        print i + '\t ' + str(count2) + " (" + str(cycles2) + ")"
                section_used_instr = {}
                section_cycles = 0

            found = False
            if re.search(search_section, line):
                found = True
            if (found == True or search_section == ''):
                print "\n"
                print "New section: " + m.group(2)
                print m.group(0)

if (found == True or search_section == ''):
    print ""
    print "Section summary:"
    print "Maximum cycles: " + str(section_cycles)
    print "Used instructions:"
    for i in section_used_instr:
        (count2, cycles2) = section_used_instr[i]
        print i + '\t ' + str(count2) + " (" + str(cycles2) + ")"

if (search_section == ''):
    print "\n\n"
    print "Result of analysis:"
    print "Maximum total cycles: " + str(total_cycles) 

    print "Total used instructions:"
    for i in total_used_instr:
        (count, cycles) = total_used_instr[i]
        print i + '\t ' + str(count) + " (" + str(cycles) + ")"
