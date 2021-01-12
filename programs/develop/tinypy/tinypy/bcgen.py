#!/bin/python2

# Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv3

import os

CORE = ['tokenize','parse','encode','py2bc']

def py2bc(cmd,mod):
    src = 'core/%s.py'%mod
    dest = 'core/%s.tpc'%mod
    
    cmd = cmd.replace('$SRC',src)
    cmd = cmd.replace('$DEST',dest)
    os.system(cmd)
    
def build_bc(opt=False):
    out = []
    for mod in CORE:
        out.append("""unsigned char tp_%s[] = {"""%mod)
        fname ="core/"+mod+".tpc"
        data = open(fname,'rb').read()
        cols = 16
        for n in xrange(0,len(data),cols):
            out.append(",".join([str(ord(v)) for v in data[n:n+cols]])+',')
        out.append("""};""")
    out.append("")
    f = open('bc.c','wb')
    f.write('\n'.join(out))
    f.close() 

for src in CORE:
    py2bc('python2 core/py2bc.py $SRC $DEST',src)
build_bc(True)
