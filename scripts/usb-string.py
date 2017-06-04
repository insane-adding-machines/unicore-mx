#!/usr/bin/env python

import numpy
import sys

#
# Use: Convert stdin input strings to UTF-16LE (USB specification) string descriptors
# How: $python usb-string.py < input-file
#
# Input file:
#  - UTF-8 strings. (seperated by newline)
#  - Empty lines are ignored
#

#
# This file is part of unicore-mx.
# Copyright (C) 2017 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
#
# usb-string.py is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# usb-string.py is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with usb-string.py.  If not, see <http://www.gnu.org/licenses/>.
#
# Special exception (same as GNU Bison)
# As a special exception, you may create a larger work that contains
# part or all of the usb-string.py parser skeleton and distribute that work
# under terms of your choice, so long as that work isn't itself a
# parser generator using the skeleton or a modified version thereof
# as a parser skeleton.  Alternatively, if you modify or redistribute
# the parser skeleton itself, you may (at your option) remove this
# special exception, which will cause the skeleton and the resulting
# usb-string.py output files to be licensed under the GNU General Public
# License without this special exception.

def print_struct(entry, i):
	utf16 = entry.encode('utf-16le') # convert to UTF-16LE byte array (return byte array)
	utf16 = numpy.asarray(bytearray(utf16))  # conver to numpy array
	utf16 = utf16.view(numpy.uint16) # uint16 (2byte view)

	print("static const struct usb_string_descriptor string_%i = {" % i)
	print("\t.bLength = USB_DT_STRING_SIZE(%i)," % len(utf16))
	print("\t.bDescriptorType = USB_DT_STRING,")
	print("\t/* %s */" % entry)
	print("\t.wData = {")

	used = 0
	SIZE = 8
	while used < len(utf16):
		# get a sub-array with maximum length <SIZE>
		f = utf16[used:used+SIZE]
		used += SIZE

		f = map(lambda b: "0x{:04x}".format(b), f) # hexify each byte
		f = ", ".join(f) # join all hex string

		if used < len(utf16):
			f += ','

		print("\t\t%s" % f)


	print("\t}")
	print("};")
	print("")

if sys.hexversion < 0x03000000:
	# stdin and stdout encoding to utf-8
	# Python3 does the trick internally
	# https://stackoverflow.com/a/2738005
	import codecs
	sys.stdin = codecs.getreader('utf-8')(sys.stdin)
	sys.stdout = codecs.getwriter('utf-8')(sys.stdout)

index = 0
for line in sys.stdin:
	line = line.strip()
	if len(line):
		print_struct(line, index)
		index += 1
