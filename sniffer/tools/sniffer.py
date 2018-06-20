#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
(C) 2012, Mariano Alvira <mar@devl.org>
(C) 2014, Oliver Hahm <oliver.hahm@inria.fr>
(C) 2015, Hauke Petersen <hauke.petersen@fu-berlin.de>
(C) 2015, Martine Lenders <mlenders@inf.fu-berlin.de>
(C) 2015, Cenk Gündoğan <cnkgndgn@gmail.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of the Institute nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
'''

from __future__ import print_function
import argparse
import sys
import re
import socket
from time import sleep, time
from struct import pack
from serial import Serial

# PCAP setup
MAGIC = 0xa1b2c3d4
MAJOR = 2
MINOR = 4
ZONE = 0
SIG = 0
SNAPLEN = 0xffff
NETWORK = 230       # 802.15.4 no FCS


def configure_interface(port, channel):
    line = ""
    iface = 0
    port.write(b'ifconfig\n')
    while True:
        line = port.readline()
        if line == '':
            print("Application has no network interface defined",
                  file=sys.stderr)
            sys.exit(2)
        match = re.search(r'^Iface +(\d+)', line.decode())
        if match is not None:
            iface = int(match.group(1))
            break

    # set channel, raw mode, and promiscuous mode
    print('ifconfig %d set chan %d' % (iface, channel), file=sys.stderr)
    print('ifconfig %d raw' % iface, file=sys.stderr)
    print('ifconfig %d promisc' % iface, file=sys.stderr)
    port.write(('ifconfig %d set chan %d\n' % (iface, channel)).encode())
    port.write(('ifconfig %d raw\n' % iface).encode())
    port.write(('ifconfig %d promisc\n' % iface).encode())


def generate_pcap(port, out):
    # count incoming packets
    count = 0
    # output overall PCAP header
    out.write(pack('<LHHLLLL', MAGIC, MAJOR, MINOR, ZONE, SIG, SNAPLEN,
                   NETWORK))
    sys.stderr.write("RX: %i\r" % count)
    while True:
        line = port.readline().rstrip()

        pkt_header = re.match(r">? *rftest-rx --- len 0x(\w\w).*",
                              line.decode())
        if pkt_header:
            now = time()
            sec = int(now)
            usec = int((now - sec) * 1000000)
            length = int(pkt_header.group(1), 16)
            out.write(pack('<LLLL', sec, usec, length, length))
            out.flush()
            count += 1
            sys.stderr.write("RX: %i\r" % count)
            continue

        pkt_data = re.match(r"(0x\w\w )+", line.decode())
        if pkt_data:
            for part in line.decode().split(' '):
                byte = re.match(r"0x(\w\w)", part)
                if byte:
                    out.write(pack('<B', int(byte.group(1), 16)))
            out.flush()


def connect(args):
    connType = args.conn_type

    conn = None
    if connType == "serial":
        # open serial port
        try:
            conn = Serial(args.tty, args.baudrate, dsrdtr=0, rtscts=0,
                          timeout=1)
        except IOError:
            print("error opening serial port", file=sys.stderr)
            sys.exit(2)
    elif connType == "socket":
        host = args.host
        port = args.port

        try:
            sock = socket.socket()
            sock.connect((host, port))
            conn = sock.makefile("r+b", bufsize=0)
        except IOError:
            print("error connecting to %s:%s" % (host, port), file=sys.stderr)
            sys.exit(2)
    else:
        print("error: unsupported connection type. Use \"serial\" or \"socket\"")
        sys.exit(2)

    return conn


class ArgparseError(Exception):
    pass


class ArgumentParser(argparse.ArgumentParser):
    def __init__(self, extra_usage=None, *args, **kwargs):
        super(ArgumentParser, self).__init__(*args, **kwargs)
        self.extra_usage = extra_usage

    def error(self, message):
        if message.startswith("argument conn_type: invalid choice: "):
            raise ArgparseError(message)
        else:
            if self.extra_usage is not None:
                print(self.extra_usage, file=sys.stderr)
            super(ArgumentParser, self).error(message)


def add_serial_args(p):
    p.add_argument("tty", type=str,
                   help="Serial port to board with sniffer application")
    p.add_argument("baudrate", type=int,
                   help="Baudrate of the serial port")


def add_general_args(p):
    p.add_argument("channel", type=int, help="Channel to sniff on")
    p.add_argument("outfile", type=str, default=None, nargs="?",
                   help="PCAP file to output to")


def main():
    p = ArgumentParser()
    sp = p.add_subparsers(dest="conn_type")
    serial_p = sp.add_parser("serial",
                             help="Parse output of sniffer application "
                                  "connected via serial line (default when no "
                                  "subcommand is used)")
    add_serial_args(serial_p)
    socket_p = sp.add_parser("socket",
                             help="Parse output of a TCP-connected sniffer "
                                  "application")
    socket_p.add_argument("host", type=str,
                          help="Host of the TCP-based sniffer application")
    socket_p.add_argument("port", type=int,
                          help="Port of the TCP-based sniffer application")
    add_general_args(p)
    try:
        args = p.parse_args()
    except ArgparseError:
        old_help = p.format_help()
        p = ArgumentParser(extra_usage=old_help)
        add_serial_args(p)
        add_general_args(p)
        p.set_defaults(conn_type="serial")
        args = p.parse_args()

    conn = connect(args)

    sleep(1)
    configure_interface(conn, args.channel)
    sleep(1)

    # figure out where to write
    if args.outfile is not None:
        sys.stderr.write('trying to open file %s\n' % args.outfile)
        outfile = open(args.outfile, 'w+b')
    else:
        if sys.version_info > (3,):
            outfile = sys.stdout.buffer
        else:
            outfile = sys.stdout

    try:
        generate_pcap(conn, outfile)
    except KeyboardInterrupt:
        conn.close()
        print()
        sys.exit(2)


if __name__ == "__main__":
    main()
