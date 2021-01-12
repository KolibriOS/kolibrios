import sys
__tinypy__ = "tinypy" in sys.version
if not __tinypy__:
    from boot import *
    __tinypy__ = False
else:
    __tinypy__ = True
    
def get_ops():
    """ Builds an value <-> opcode name dictionary """
    li = ["EOF","ADD","SUB","MUL","DIV","POW","BITAND","BITOR","CMP","GET", \
          "SET","NUMBER","STRING","GGET","GSET","MOVE","DEF","PASS",  \
          "JUMP","CALL","RETURN","IF","DEBUG","EQ","LE","LT","DICT",  \
          "LIST","NONE","LEN","LINE","PARAMS","IGET","FILE","NAME",   \
          "NE","HAS","RAISE","SETJMP","MOD","LSH","RSH","ITER","DEL", \
          "REGS","BITXOR", "IFN", "NOT", "BITNOT"]
    dic = {}
    for i in li:
        dic[li.index(i)] = i
    return dic

def isupper(x):
    return ord(x) >= ord("A") and ord(x) <= ord("Z")

def pad(s, n):
    p = ""
    if n < 0:
        m = -n - len(s)
        if m > 0: p = " " * m
        return p + s
    m = n - len(s)
    if m > 0: p = " " * m
    return s + p

def funpack(bytes):
    if not __tinypy__:
        import struct
        return struct.unpack("d", bytes)[0]
    def eat(x, bit):
        y = int(x / 2 ** bit)
        x -= y * 2 ** bit
        return x, y
    x = 0
    for i in range(8):
        x += ord(bytes[i]) * 2 ** (i * 8)
    x, sign = eat(x, 63)
    x, exponent = eat(x, 52)
    x, mantissa1 = eat(x, 31)
    x, mantissa2 = eat(x, 0)
    mantissa = mantissa1 * 2 ** 31 + mantissa2
    sign = sign * -2 + 1
    x = sign * 2 ** (exponent - 1023) * (1 + mantissa / 2 ** 52)
    return x

def text(x, ip, bc):
    return "".join([chr(c) for c in bc[ip:ip+x]])

def trim(x):
    txt = []
    for c in x:
        if ord(c):
            txt.append(c)
    return "".join(txt)

def disassemble(bc):    
    bc = [ord(x) for x in bc]
    asmc = []
    ip = 0
    names = get_ops()
    while ip < len(bc):
        i, a, b, c = bc[ip:ip + 4]
        line = ""
        line += pad(names[i], 10) + ":" 
        line += " " + pad(str(a), -3)
        line += " " + pad(str(b), -3)
        line += " " + pad(str(c), -3)
        ip += 4
        if names[i] == "LINE":
            n = a * 4
            line += " \"" + text(n,ip,bc) + "\""
            line = trim(line)
            ip += n
        elif names[i] == "STRING":
            n = b * 256 + c
            line += " \"" + text(n,ip,bc) + "\""
            line = trim(line)
            ip += (int(n / 4) + 1) * 4 
        elif names[i] == "NUMBER":   
            f = funpack(text(8,ip,bc))
            line += " " + str(f)
            ip += 8
        asmc.append(line)
    asmc = "\n".join(asmc)
    return asmc
    
if __name__ == "__main__":
    bc = load(ARGV[1])
    asmc = disassemble(bc)
    print(asmc)
