#!/usr/bin/env python

import sys

indent = b""

with open(sys.argv[1], 'rb') as fin:
    with open(sys.argv[2], 'wb') as fout:
        for line in fin:
            if line.endswith(b"end\r\n"):
                indent = indent[:-2]
            fout.write(indent)
            fout.write(line)
            if line.endswith(b"begin\r\n"):
                indent = indent + b"  "
