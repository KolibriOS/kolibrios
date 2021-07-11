import re
import sys
if len(sys.argv) < 2 or sys.argv[1].strip() == '':
    print('error: no input file\nusage: {:s} <file>'.format(sys.argv[0]))
    exit(-1)
inp_path = sys.argv[1].strip()
out_path = inp_path + '.map'
inp = open(inp_path,'r')
inp_text = inp.read()
inp.close()
regex1 = re.compile(r"[\S]+\:\s0x[\da-fA-F]*")
res = re.findall(regex1, inp_text)
#print(res)
out = open(out_path, 'w')
for ln in res:
    splt = ln.split()
    sym_addr = int(splt[1],16)
    sym_name = splt[0][:-1]
    out.write('{:016x} {:s}\n'.format(sym_addr, sym_name))
out.close()
