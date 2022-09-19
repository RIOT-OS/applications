#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
(C) 2012, Mariano Alvira <mar@devl.org>
(C) 2014, Oliver Hahm <oliver.hahm@inria.fr>
(C) 2015, Hauke Petersen <hauke.petersen@fu-berlin.de>
(C) 2015, Martine Lenders <mlenders@inf.fu-berlin.de>
(C) 2015, Cenk Gündoğan <cnkgndgn@gmail.com>
(C) 2019, Freie Universität Berlin (by Martine Lenders <m.lenders@fu-berlin.de)

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
import socket
from time import sleep, time
from struct import pack, unpack
from serial import Serial

END_BYTE = 0xc0
ESC_BYTE = 0xdb
END_ESC_BYTE = 0xdc
ESC_ESC_BYTE = 0xdd

# PCAP setup
MAGIC = 0xa1b2c3d4
MAJOR = 2
MINOR = 4
ZONE = 0
SIG = 0
SNAPLEN = 0xffff
NETWORK = 230       # 802.15.4 no FCS

DEFAULT_BAUDRATE = 115200


def configure_interface(port, channel):
    port.write(bytes([channel]))


class DecodeError(Exception):
    pass


def read_byte(port):
    byte = port.read()
    while not byte:
        byte = port.read(1)
    return byte[0]


def decode(port, num=None):
    byte = 0
    res = bytearray()
    while (byte != END_BYTE) and ((num is None) or (num > 0)):
        byte = read_byte(port)
        if byte == END_BYTE:
            return bytes(res)
        elif byte == ESC_BYTE:
            byte = read_byte(port)
            if byte == END_ESC_BYTE:
                res.append(END_BYTE)
            elif byte == ESC_ESC_BYTE:
                res.append(ESC_BYTE)
            else:
                raise DecodeError("Unexpected byte \"{:02x}\" "
                                  "after escape received".format(byte))
        else:
            res.append(byte)
        if num is not None:
            num -= 1
    return bytes(res)


def find_next_end(port):
    while read_byte(port) != END_BYTE:
        pass


def generate_pcap(port, out):
    # count incoming packets
    count = 0
    # output overall PCAP header
    out.write(pack('<LHHLLLL', MAGIC, MAJOR, MINOR, ZONE, SIG, SNAPLEN,
                   NETWORK))
    sys.stderr.write("RX: %i\r" % count)
    # start after first END_BYTE
    find_next_end(port)
    while True:
        try:
            # read packet "header" in network byte order:
            #
            #  0                   1                   2                   3
            #  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
            # +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            # |                         packet length                         |
            # +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            # |      LQI      |                    padding                    |
            # +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            # |                                                               |
            # +                       Timestamp (in us)                       +
            # |                                                               |
            # +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            length, _, _ = unpack("!LB3xQ", decode(port, 16))

            if length:
                now = time()
                sec = int(now)
                usec = int((now - sec) * 1000000)
                out.write(pack('<LLLL', sec, usec, length, length))
                out.flush()
                count += 1
                sys.stderr.write("RX: %i\r" % count)
            else:
                continue

            # don't limit read by `length` in case the data is corrupted
            out.write(decode(port))
            out.flush()
        except DecodeError as exc:
            sys.stderr.write(exc + "\r")
            find_next_end(port)


def connect(args):
    conn = None
    if args.conn.startswith("/dev/tty") or args.conn.startswith("COM"):
        # open serial port
        try:
            conn = Serial(args.conn, args.baudrate, dsrdtr=0, rtscts=0,
                          timeout=1)
        except IOError:
            print("error opening serial port %s" % args.conn, file=sys.stderr)
            sys.exit(2)
    else:
        try:
            port = args.conn.split(":")[-1]
            host = args.conn[:-(len(port)+1)]
            port = int(port)
        except (IndexError, ValueError):
            print("Can't parse host:port pair %s" % args.conn, file=sys.stderr)
            sys.exit(2)
        try:
            sock = socket.socket()
            sock.connect((host, port))
            conn = sock.makefile("r+b", bufsize=0)
        except IOError:
            print("error connecting to %s:%s" % (host, port), file=sys.stderr)
            sys.exit(2)
    return conn


def main():
    if sys.version_info > (3,):
        default_outfile = sys.stdout.buffer
    else:
        default_outfile = sys.stdout
    p = argparse.ArgumentParser()
    p.add_argument("-b", "--baudrate", type=int, default=DEFAULT_BAUDRATE,
                   help="Baudrate of the serial port (only evaluated "
                        "for non TCP-terminal, default: %d)" %
                        DEFAULT_BAUDRATE)
    p.add_argument("conn", metavar="tty/host:port", type=str,
                   help="Serial port or TCP (host, port) tuple to "
                        "terminal with sniffer application")
    p.add_argument("channel", type=int, help="Channel to sniff on")
    p.add_argument("outfile", type=argparse.FileType("w+b"),
                   default=default_outfile, nargs="?",
                   help="PCAP file to output to (default: stdout)")
    args = p.parse_args()

    conn = connect(args)

    sleep(1)
    configure_interface(conn, args.channel)
    sleep(1)

    try:
        generate_pcap(conn, args.outfile)
    except KeyboardInterrupt:
        conn.close()
        print()
        sys.exit(2)


if __name__ == "__main__":
    main()
