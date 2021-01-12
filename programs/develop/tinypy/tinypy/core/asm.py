def get_ops():
    """ Builds an opcode name <-> value dictionary """
    li = ["EOF","ADD","SUB","MUL","DIV","POW","BITAND","BITOR","CMP","GET", \
          "SET","NUMBER","STRING","GGET","GSET","MOVE","DEF","PASS",  \
          "JUMP","CALL","RETURN","IF","DEBUG","EQ","LE","LT","DICT",  \
          "LIST","NONE","LEN","LINE","PARAMS","IGET","FILE","NAME",   \
          "NE","HAS","RAISE","SETJMP","MOD","LSH","RSH","ITER","DEL", \
          "REGS","BITXOR", "IFN", "NOT", "BITNOT"]
    dic = {}
    for i in li:
        dic[i] = li.index(i)
    return dic

def prepare(x):
    """ Prepares the line for processing by breaking it into tokens,
        removing empty tokens and stripping whitespace """
    try:
        ind = x.index('"')
    except:
        ind = -1
    if ind != -1:
        d = x[ind:]
        x = x[:ind]
    x = x.split(' ')
    tmp = []
    final = []
    for i in x:
        if i:
            if i[0] != ':':
                tmp.append(i)
    for i in tmp[:4]:
        final.append(i)
    if not d:
        d = "".join(tmp[4:])
    final.append(d.strip())
    return final

def dequote(x):
    """ Removes outermost quotes from a string, if they exist """    
    if x[0] == '"' and x[len(x)-1] == '"':
        return x[1:len(x)-1]
    return x

def assemble(asmc):    
    asmc = asmc.strip()
    asmc = asmc.split('\n')
    bc = []
    ops = get_ops()
    for line in asmc:
        current = prepare(line)
        i,a,b,c,d = current
        a = int(a)
        b = int(b)
        c = int(c)
        bc.append(chr(ops[i]))
        bc.append(chr(a))
        bc.append(chr(b))
        bc.append(chr(c))
        if i == "LINE":
            n = a * 4
            d = dequote(d)
            text = d
            text += chr(0) * (n - len(d))
            bc.append(text)
        if i == "STRING":
            d = dequote(d)
            text = d + "\0"*(4-len(d)%4)
            bc.append(text)
        elif i == "NUMBER":
            d = int(d)
            bc.append(fpack(d))
    bc = "".join(bc)
    return bc
    
if __name__ == '__main__':
    asmc = load(ARGV[1])
    bc = assemble(asmc)
    save(ARGV[2], bc)
