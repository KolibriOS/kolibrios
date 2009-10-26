#! /usr/bin/env python3
# -*- coding: utf-8 -*-

# Launch this in following way:
# readelf --syms <file> | sysmbold2bdg.py ><file.dbg>

#8: 00000004     4 OBJECT  GLOBAL DEFAULT  COM main_group_list

# Skip 3 lines
input()
input()
input()

while True:
	try:
		s = input().split()
	except:
		break
	try:
		print('0x'+s[1], s[7])
	except:
		pass
