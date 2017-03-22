#!/usr/bin/env python
import argparse

def _gen_binary(enable_frp, size, out):
	'''Create a binary with all NULLs, and the last byte 0 or 1'''

	with open(out, "wb") as f:
		f.seek(size - 1) #http://stackoverflow.com/a/3377909

		# Write the last byte as zero or one
		# FRP = Factory Reset Protection
		# Last byte = 0 => User is not allowed to do oem unlock
		if enable_frp:
			f.write('\0')
		else:
			f.write('\1')

def _get_parser():
	'''Parsing command line arguments'''

	parser = argparse.ArgumentParser(description='Generate RP2 binaries')
	parser.add_argument('--enable_frp', action='store_true',
						help='Zero out the last byte of the partition')
	parser.add_argument('-s', '--size', default='4194304', type=int,
						help='Size of the RP2 binary')
	parser.add_argument('-o', '--output', default='rp2.bin',
						help='Name of the generated binary')

	return parser

if __name__ == '__main__':
	parser = _get_parser()
	args = parser.parse_args()
	args = vars(args) # This makes args a dictionary
	_gen_binary(args['enable_frp'], args['size'], args['output'])
