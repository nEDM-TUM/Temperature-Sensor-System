#!/usr/bin/python2

# usage:  avr-objdump main.elf | ./wcet.py [section_search_pattern]

import sys
import re

startaddress = int("10e", 16)
stopaddress = int("fffffff", 16)

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

opbranches = {}
f = open('opcodes_cond_branch.lst', 'r')
for line in f:
    m = re.search('^([a-z]*)\s+([0-9])+\s+([0-9])+.*$',line)
    if m:
        #print 'instr ' + m.group(1) + " time " + m.group(2)
        opbranches[m.group(1)] = (int(m.group(2)), int(m.group(3)))

opjumps = {}
f = open('opcodes_jumps.lst', 'r')
for line in f:
    m = re.search('^([a-z]*)\s+([0-9])+\s+.*$',line)
    if m:
        #print 'instr ' + m.group(1) + " time " + m.group(2)
        opjumps[m.group(1)] = int(m.group(2))

opskips = {}
f = open('opcodes_skips.lst', 'r')
for line in f:
    m = re.search('^([a-z]*)\s+([0-9])+\s+([0-9])+\s+([0-9])+.*$',line)
    if m:
        #print 'instr ' + m.group(1) + " time " + m.group(2)
        opskips[m.group(1)] = (int(m.group(2)), int(m.group(4)))

opsbig = {}
f = open('opcodes_big.lst', 'r')
for line in f:
    m = re.search('^([a-z]*)\s+([0-9])+.*$',line)
    if m:
        #print 'instr ' + m.group(1) + " time " + m.group(2)
        opsbig[m.group(1)] = int(m.group(2))

# load code:
print "Loading code..."
code = {}
previous_address = -1
for line in sys.stdin:
    m = re.search('^\s*([0-9a-f]+):(?:\s*[0-9a-f])*\s+([a-z]+)\s+(.*)$', line)
    if m:
        #print line
        address = int(m.group(1), 16)
        if (previous_address == -1):
            previous_address = address
        instruction = m.group(2)
        time1 = optime[instruction] # for non complex instructions we get the time here
        time2 = 0 # this is for complex instructions
        destination = 0
        size = 2
        instr_type = "norm"
        if instruction in opbranches:
            m = re.search('^\s*([0-9a-f]+):(?:\s*[0-9a-f])*\s+([a-z]+)\s+.*;\s+0x([0-9a-f]+)\s.*$', line)
            destination = int(m.group(3), 16)
            (time1, time2) = opbranches[instruction]
            instr_type = "br"
        if instruction in opjumps:
            m = re.search('^\s*([0-9a-f]+):(?:\s*[0-9a-f])*\s+([a-z]+)\s+.*;\s+0x([0-9a-f]+)\s.*$', line)
            destination = int(m.group(3), 16)
            time1 = opjumps[instruction]
            instr_type = "jmp"
        if instruction in opskips:
            destination = address + 2  # default skip: two words. it is hard to detect how far to skip...
            # idea: if we have an instruction with length !=2 just correct it
            (time1, time2) = opskips[instruction]
            instr_type = "skip"

        # now we know the size of the previous instruction:
        if ((address - previous_address <=4) and (address != previous_address)):
            (x1, x2, x3, x4, x5, x6) = code[previous_address]
            code[previous_address] = (x1, x2, address-previous_address, x4, x5, x6)
        previous_address = address

        code[address] = (instruction, instr_type, size, time1, time2, destination)
    else:
        m = re.search('^\s*([0-9a-f]+)\s+\<(.*)\>:$', line)
        if m:
            if re.search(search_section, line):
                print "found section: " + m.group(2)
                startaddress = int(m.group(1), 16)






def wcet(startaddr, stopaddr, depth):
    if depth > 10:
        print "ERROR: Maximum recursion depth reached"
        return 0
    addr = startaddr
    cycles = 0
    instruction = ""
    while (addr <= stopaddr and addr >= startaddr and (instruction != "reti" and instruction != "ret")):
        print hex(addr) + " : " + str(code [addr])
        (instruction, instr_type, size, time1, time2, destination) = code[addr]
        if (instr_type == "norm"):
            cycles = cycles + time1
            addr = addr + size
        if (instr_type == "jmp"):
            cycles = cycles + time1
            addr = destination
        if (instr_type == "br"):
            if addr > destination:
                print "ERROR: Loop detected!! -> assuming only single pass"
                addr = addr + size
            else:
                print "branching into depth " + str(depth + 1) + " Time (without jump) is " + str(cycles) 
                return max(cycles + time2 + wcet(destination, stopaddr, depth + 1), cycles + time1 + wcet(addr + size, stopaddr, depth + 1))
        if (instr_type == "skip"):
            cycles = cycles + time1
            (_,_, size_next,_,_,_) = code[addr + size]
            addr = addr + size + size_next
    print "branch from " + hex(startaddr) + " finished. Branch time was: " + str(cycles) + ". depth was " + str(depth)
    return cycles


wcetresult =  str(wcet(startaddress, stopaddress, 0))
print "RESULT: " + wcetresult

#total_used_instr = {}
#total_cycles = 0
#section_used_instr = {}
#section_cycles = 0
#for line in sys.stdin:
#    m = re.search('^\s*([0-9a-f]+):(?:\s*[0-9a-f])*\s+([a-z]+)\s+(.*)$', line)
#    #m = re.search('^\s.*$', line)
#    if m:
#        #print m.group(0)
#        instr = m.group(2)
#        current_optime = optime[instr]
#        if (found == True or search_section == ''):
#            print str(section_cycles) + " + " + str(current_optime) + " " + m.group(1) + ": " + m.group(2) + " \t" + m.group(3)
#        total_cycles = total_cycles + current_optime
#        section_cycles = section_cycles + current_optime
#        try:
#            (count, cycles) = total_used_instr[instr]
#            total_used_instr[instr] = (count + 1 , cycles + current_optime)
#        except KeyError:
#            total_used_instr[instr] = (1, current_optime)
#        try:
#            (count, cycles) = total_used_instr[instr]
#            section_used_instr[instr] = (count + 1 , cycles + current_optime)
#        except KeyError:
#            section_used_instr[instr] = (1, current_optime)
#    else:
#        m = re.search('^\s*([0-9a-f]+)\s+\<(.*)\>:$', line)
#        if m:
#            if ( total_cycles != 0 ):
#                if (found == True or search_section == ''):
#                    print ""
#                    print "Section summary:"
#                    print "Maximum cycles: " + str(section_cycles)
#                    print "Used instructions:"
#                    for i in section_used_instr:
#                        (count2, cycles2) = section_used_instr[i]
#                        print i + '\t ' + str(count2) + " (" + str(cycles2) + ")"
#                section_used_instr = {}
#                section_cycles = 0
#
#            found = False
#            if re.search(search_section, line):
#                found = True
#            if (found == True or search_section == ''):
#                print "\n"
#                print "New section: " + m.group(2)
#                print m.group(0)
#
#if (found == True or search_section == ''):
#    print ""
#    print "Section summary:"
#    print "Maximum cycles: " + str(section_cycles)
#    print "Used instructions:"
#    for i in section_used_instr:
#        (count2, cycles2) = section_used_instr[i]
#        print i + '\t ' + str(count2) + " (" + str(cycles2) + ")"
#
#if (search_section == ''):
#    print "\n\n"
#    print "Result of analysis:"
#    print "Maximum total cycles: " + str(total_cycles) 
#
#    print "Total used instructions:"
#    for i in total_used_instr:
#        (count, cycles) = total_used_instr[i]
#        print i + '\t ' + str(count) + " (" + str(cycles) + ")"
