#!/usr/bin/env python

import os
import sys
import struct
import binascii

def usage():
    print "Usage: %s <firmware.LOD> <firmware revision> [output.rp4]" % (sys.argv[0])
    print ""
    print "    Will pack <firmware.LOD> into [output.rp4] with correct header"
    print "    Resulting file will be rp4.bin if [output.rp4] is not specified"
    sys.exit(1)

if __name__ == "__main__":

    if len(sys.argv) < 3 or len(sys.argv) > 4:
        usage()

    fw_file = sys.argv[1]
    fw_rev  = sys.argv[2]

    if len(sys.argv) > 3:
        rp4_file = sys.argv[3]
    else:
        rp4_file = 'rp4.bin'

    if len(fw_rev) > 8:
        print "firmware revision string should be less than 8 byte"
        sys.exit(2)

    statinfo = os.stat(fw_file)
    with open(fw_file, 'r') as fw:
        fwdata = fw.read()
        crcsum = binascii.crc32(fwdata) & 0xffffffff

    # NOTE: header aligned to be 4096-byte (page size)
    # adding fields need to change the padding number of bytes
    # struct sata_fw_header {
    #      uint8_t magic[16];   /* 16-byte "NVIDIA__SATA__FW" */
    #      uint8_t fw_rev[8];   /* 8-byte firmware revision string */
    #      uint32_t fw_offset;
    #      uint32_t fw_size;
    #      uint32_t fw_crc32;
    # }
    fmt = struct.Struct('<16s8sIII')
    hdr = bytearray(b'\x00'*4096)
    fmt.pack_into(hdr, 0, "NVIDIA__SATA__FW", fw_rev, len(hdr), statinfo.st_size, crcsum)

    with open(rp4_file, 'w') as rp4:
        rp4.write(hdr)
        rp4.write(fwdata)

    print "%s file written" % rp4_file
