#!/usr/bin/env python

from optparse import OptionParser

import os
import sys

def check_inputs(options, args):
	if (not len(args) == 1):
		parser.print_usage()
		return False
		
	if (not options.output):
		directory = os.path.dirname(args[0])
		basename = os.path.basename(args[0])
		
		basename = basename.rsplit(".", 1)[0] + ".db"
		
		options.output = os.path.join(directory, basename)
		
	return True

def parse(input_file, output_file, options):
	for line in input_file:
		line = line.strip()
		
		if (not line or line[0] == "#"):
			continue
			
		(first_half, second_half) = line.split("->")
		name = first_half.split("[")[0].strip()
		type = second_half.split("/")[0].strip()
		
		to_write = """\
record(%s, %s)
{
	field(DTYP, "asynInt32")
	field(SCAN, "I/O Intr")
	field(INP, "@asyn($(PORT), 0, 0)%s")
}

"""
		
		if type == "Bool":
			output_file.write(to_write % ("bi", options.prefix + name, name))
		else:
			output_file.write(to_write % ("ai", options.prefix + name, name))
	
	
if __name__ == "__main__":
	usage = "Usage: %prog specification_file [options]"
	
	parser = OptionParser(usage=usage)
	
	parser.add_option("-p", "--prefix", metavar="PREFIX",
	                  action="store", type="string", dest="prefix", default="$(P)$(R)",
	                  help="Record name prefix [default: %default]")
	
	parser.add_option("-o", "--output", metavar="FILE",
	                  action="store", type="string", dest="output", default=None,
	                  help="Output database filename. By default, uses the input file to determine output name")
	
	(options, args) = parser.parse_args()
						
	if check_inputs(options, args):
		with open(args[0], "r") as input_file:
			with open(options.output, "w") as output_file:
				parse(input_file, output_file, options)
	
	
