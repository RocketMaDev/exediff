#!/bin/python
import sys
if len(sys.argv) == 1:
    print('set a file as arg to flatten')
    exit(1)
with open(sys.argv[1], 'rb') as f:
    binary = f.read()
for b in binary:
    print(hex(b))
