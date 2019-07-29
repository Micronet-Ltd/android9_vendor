# Copyright (c) 2015, 2017, The Linux Foundation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 and
# only version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

import datetime
import logging
import logging.handlers
import os
import struct
import sys

from optparse import OptionParser

count = 0
address = []
data = []


def log_init(name, path, filename):
    # Set up logger
    logger = logging.getLogger(name)
    logger.setLevel(logging.INFO)

    # Add the log message handler to the logger
    if filename is not None:
        handler = logging.FileHandler(path + '/' + filename, mode='w')
    else:
        handler = logging.StreamHandler(sys.stdout)

    logger.addHandler(handler)
    return logger


def add_addr(base, offset, length):
    for i in range(0, length):
        addr = base + offset + (i * 4)
        address.append(addr)


def add_loop_addr(loop_nr, loop_count):
    for i in range(0, loop_count):
        for j in range(len(address) - loop_nr, len(address)):
            address.append(address[j])


def read_data(data_pt):
    nr = count
    while nr > 0:
        word = data_pt.read(4)
        if len(word) != 4:
            break

        val = struct.unpack('<L', word)[0]
        data.append(val)
        nr = nr - 1

    return nr


def read_config(config_pt):
    list_nr = []
    list_nr.append(0)
    offset = 0
    base = 0

    if options.version is None:
        address_descriptor = 0x1 << 31
        link_descriptor = 0
        loop_descriptor = None
        rd_mod_wr_descriptor = None
        dcc_write_ind = None
        link_second_arg = 8
        # We return zero and fail
        on_zero_link_len = 0
        track_len = 0
    else:
        address_descriptor = 0
        link_descriptor = 0x3 << 30
        loop_descriptor = 0x1 << 30
        rd_mod_wr_descriptor = 0x1 << 31
	dcc_write_ind = 0x1 << 28
        link_second_arg = 7
        #indicates end of list
        on_zero_link_len = -1
        #word size
        track_len = 4

    if options.config_offset is not None:
        config_pt.seek(int(options.config_offset, 16), 1)

    while True:
        word = config_pt.read(4)
        if len(word) != 4:
            break

        val = struct.unpack('<L', word)[0]

        if val == 0:
            break

        descriptor = val & (0x3 << 30)
	read_write_ind = val & (0x1 << 28)

	if read_write_ind == dcc_write_ind:
            config_pt.seek(8, 1)
        elif descriptor == address_descriptor:
            base = ((val & 0x0FFFFFFF) << 4)
            offset = 0
            length = 1
        elif descriptor == link_descriptor:
            for i in range(0, 2):
                offset = offset + (val & 0xFF) * 4 + (length - 1) * track_len
                val = val >> 8

                length = (val & 0x7f)
                val = val >> link_second_arg

                if length != 0:
                    list_nr.append(length + list_nr[- 1])
                    add_addr(base, offset, length)
                else:
                    if (i == 0 ):
                        return list_nr[on_zero_link_len]
                    else:
                        offset = 0
        elif descriptor == loop_descriptor:
            loop_offset = val & 0x1FFF
            loop_count = (val & 0xFFFE000) >> 13

            if loop_offset == 0:
                continue

            loop_nr = list_nr[-1] - list_nr[-loop_offset]
            list_nr.append(loop_nr * loop_count + list_nr[-1])
            add_loop_addr(loop_nr, loop_count)

        elif descriptor == rd_mod_wr_descriptor:
            '''
            Skip over mask and value of rd_mod_wr.
            There is no gaurantee of this being actually written
            and we never read the value back to confirm.
            '''
            config_pt.seek(8, 1)

    return list_nr[-1]


def new_linked_list(config_pt):
    word = config_pt.read(4)

    if len(word)!= 4:
        return False
    else:
        val = struct.unpack('<L', word)[0]
        if val != 0:
            config_pt.seek(-4, 1)
            return True
        else:
            return False


def dump_regs_json(options):
    log.info("Dumping regs in JSON format in \'{0}\' file.".format(options.outfile))
    parsed_data.info("{")
    parsed_data.info("\t\"version\": 1,")
    parsed_data.info("\t\"timestamp\": \"{0}\",".format(datetime.date.today().strftime('%m/%d/%y')))
    parsed_data.info("\t\"generator\": \"Linux DCC Parser\",")
    parsed_data.info("\t\"chip\": {")
    parsed_data.info("\t\t\"name\": \"{0}\",".format(options.chipname))
    parsed_data.info("\t\t\"version\": \"{0}\"".format(options.chipversion))
    parsed_data.info("\t},")
    parsed_data.info("\t\"registers\": [")
    for addr, val in zip(address, data):
        parsed_data.info("\t\t{{ \"address\": \"0x{0:08x}\", \"value\": \"0x{1:08x}\" }},".format(addr, val))
    parsed_data.info("\t]")
    parsed_data.info("}")
    return


def dump_regs_xml(options):
    log.info("Dumping regs in XML format in \'{0}\' file.".format(options.outfile))
    parsed_data.info("<?xml version=\"1.0\" encoding=\"utf-8\"?>")
    parsed_data.info("<hwioDump version=\"1\">")
    parsed_data.info("\t<timestamp>{0}</timestamp>".format(datetime.date.today().strftime('%m/%d/%y')))
    parsed_data.info("\t<generator>Linux DCC Parser</generator>")
    parsed_data.info("\t<chip name=\"{0}\" version=\"{1}\">".format(options.chipname, options.chipversion))
    for addr, val in zip(address, data):
        parsed_data.info("\t\t<register address=\"0x{0:08x}\" value=\"0x{1:08x}\" />".format(addr, val))
    parsed_data.info("\t</chip>")
    parsed_data.info("</hwioDump>")
    return


def dump_regs(options):
    if not address:
        log.error('No configuration found in SRAM!!')
        sys.exit(1)

    if options.json is True:
        dump_regs_json(options)
    else:
        dump_regs_xml(options)


if __name__ == '__main__':
    usage = 'usage: %prog [options to print]. Run with --help for more details'
    parser = OptionParser(usage)
    parser.add_option('-s', '--sram-file', dest='sramfile',
                      help='sram image path')
    parser.add_option('-a', '--atb-file', dest='atbfile', help='atb image path')
    parser.add_option('-j', '--json', action='store_true',
                      help='output in JSON format')
    parser.add_option('-o', '--out-dir', dest='outdir', help='output dir path')
    parser.add_option('-f', '--output-file', dest='outfile',
                      help='output filename')
    parser.add_option('-l', '--log-file', dest='logfile', help='Log filename')
    parser.add_option('', '--chip-name', dest='chipname', help='chip name')
    parser.add_option('', '--chip-version', dest='chipversion',
                      help='chip version')
    parser.add_option('--v2', dest='version', action="store_const", const='2',
                      help='DCC driver version 2')
    parser.add_option('--config-offset', dest='config_offset',
                      help='Start offset for DCC configuration')

    (options, args) = parser.parse_args()

    args = ''
    for arg in sys.argv:
        args = args + arg + ' '

    if options.outdir:
        if not os.path.exists(options.outdir):
            print ('!!! Out directory does not exist. Creating...')
            try:
                os.makedirs(options.outdir)
            except:
                print ("Failed to create %s. You probably don't have permissions there. Bailing." % options.outdir)
                sys.exit(1)
    else:
        options.outdir = '.'

    if options.json:
        ext = '.json'
    else:
        ext = '.xml'

    if options.outfile is None:
        options.outfile = 'dcc_captured_data{0}'.format(ext)

    log = log_init('LOG', options.outdir, options.logfile)

    log.info("Data Capture and Compare(DCC) parser.")

    if options.sramfile is None:
        log.error("No SRAM image file given! Exiting...")
        parser.print_usage()
        sys.exit(1)

    try:
        sram_file = open(options.sramfile, 'rb')
    except:
        log.error("could not open path {0}".format(options.sramfile))
        log.error("Do you have read permissions on the path?")
        sys.exit(1)

    if options.atbfile is not None:
        try:
            atb_file = open(options.atbfile, 'rb')
        except:
            log.error("could not open path {0}".format(options.atbfile))
            log.error("Do you have read permissions on the path?")
            sys.exit(1)

    count = 0
    while True:
        count = read_config(sram_file)

        if options.atbfile is None:
            atb_file = sram_file

        if read_data(sram_file):
            log.error('Couldn\'t read complete data.')
            break

        if new_linked_list(sram_file) is False:
            parsed_data = log_init('PARSED_DATA', options.outdir, options.outfile)
            dump_regs(options)
            break

    sram_file.close()

    if options.atbfile is not None:
        atb_file.close()

    sys.stderr.flush()
