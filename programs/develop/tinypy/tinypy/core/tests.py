import sys
is_tinypy = "tinypy" in sys.version
if not is_tinypy:
    from boot import *
import asm
import disasm

################################################################################
RM = 'rm -f '
VM = './vm '
TINYPY = './tinypy '
TMP = 'tmp.txt'
if '-mingw32' in ARGV or "-win" in ARGV:
    RM = 'del '
    VM = 'vm '
    TINYPY = '..\\tinypy '
    TMP = 'tmp.txt'
    #TMP = 'stdout.txt'
SANDBOX = '-sandbox' in ARGV
def system_rm(fname):
    system(RM+fname)

################################################################################
#if not is_tinypy:
    #v = chksize()
    #assert (v < 65536)

################################################################################
def t_show(t):
    if t.type == 'string': return '"'+t.val+'"'
    if t.type == 'number': return t.val
    if t.type == 'symbol': return t.val
    if t.type == 'name': return '$'+t.val
    return t.type
def t_tokenize(s,exp=''):
    import tokenize
    result = tokenize.tokenize(s)
    res = ' '.join([t_show(t) for t in result])
    #print(s); print(exp); print(res)
    assert(res == exp)

if __name__ == '__main__':
    t_tokenize("234",'234')
    t_tokenize("234.234",'234.234')
    t_tokenize("phil",'$phil')
    t_tokenize("_phil234",'$_phil234')
    t_tokenize("'phil'",'"phil"')
    t_tokenize('"phil"','"phil"')
    t_tokenize("'phil' x",'"phil" $x')
    t_tokenize("#comment","")
    t_tokenize("and","and")
    t_tokenize("=","=")
    t_tokenize("()","( )")
    t_tokenize("(==)","( == )")
    t_tokenize("phil=234","$phil = 234")
    t_tokenize("a b","$a $b")
    t_tokenize("a\nb","$a nl $b")
    t_tokenize("a\n    b","$a nl indent $b dedent")
    t_tokenize("a\n    b\n        c", "$a nl indent $b nl indent $c dedent dedent")
    t_tokenize("a\n    b\n    c", "$a nl indent $b nl $c dedent")
    t_tokenize("a\n    b\n        \n      c", "$a nl indent $b nl nl indent $c dedent dedent")
    t_tokenize("a\n    b\nc", "$a nl indent $b nl dedent $c")
    t_tokenize("a\n  b\n    c\nd", "$a nl indent $b nl indent $c nl dedent dedent $d")
    t_tokenize("(\n  )","( )")
    t_tokenize("  x","indent $x dedent")
    t_tokenize("  #","")
    t_tokenize("None","None")


################################################################################

def t_lisp(t):
    if t.type == 'block':
        return """{%s}"""%' '.join([t_lisp(tt) for tt in t.items])
    if t.type == 'statement':
        return """%s;"""%' '.join([t_lisp(tt) for tt in t.items])
    if t.items == None: return t.val
    args = ''.join([" "+t_lisp(tt) for tt in t.items])
    return "("+t.val+args+")"

def t_parse(s,ex=''):
    import tokenize, parse
    r = ''
    tokens = tokenize.tokenize(s)
    tree = parse.parse(s,tokens)
    r = t_lisp(tree)
    #print(s); print(ex); print(r)
    assert(r==ex)

def t_unparse(s):
    import tokenize, parse
    ok = False
    try:
        tokens = tokenize.tokenize(s)
        tree = parse.parse(s,tokens)
    except:
        ok = True
    assert(ok == True)


if __name__ == '__main__':
    t_parse('2+4*3', '(+ 2 (* 4 3))')
    t_parse('4*(2+3)', '(* 4 (+ 2 3))')
    t_parse('(2+3)*4', '(* (+ 2 3) 4)')
    t_parse('1<2', '(< 1 2)')
    t_parse('x=3', '(= x 3)')
    t_parse('x = 2*3', '(= x (* 2 3))')
    t_parse('x = y', '(= x y)')
    t_parse('2,3', '(, 2 3)')
    t_parse('2,3,4', '(, 2 3 4)')
    t_parse('[]', '([])')
    t_parse('[1]', '([] 1)')
    t_parse('[2,3,4]', '([] 2 3 4)')
    t_parse('print(3)', '($ print 3)')
    t_parse('print()', '($ print)')
    t_parse('print(2,3)', '($ print 2 3)')
    t_parse('def fnc():pass', '(def fnc (():) pass)')
    t_parse('def fnc(x):pass', '(def fnc ((): x) pass)')
    t_parse('def fnc(x,y):pass', '(def fnc ((): x y) pass)')
    t_parse('x\ny\nz', '(; x y z)')
    t_parse('return x', '(return x)')
    t_parse('print(test(2,3))', '($ print ($ test 2 3))')
    t_parse('x.y', '(. x y)')
    t_parse('get(2).x', '(. ($ get 2) x)')
    t_parse('{}', '({})')
    t_parse('True', 'True')
    t_parse('False', 'False')
    t_parse('None', 'None')
    t_parse('while 1:pass', '(while 1 pass)')
    t_parse('for x in y:pass', '(for x y pass)')
    t_parse('print("x")', '($ print x)')
    t_parse('if 1: pass', '(if (elif 1 pass))')
    t_parse('x = []', '(= x ([]))')
    t_parse('x[1]', '(. x 1)')
    t_parse('import sdl', '(import sdl)')
    t_parse('2 is 3', '(is 2 3)')
    t_parse('2 is not 3', '(isnot 2 3)')
    t_parse('x is None', '(is x None)')
    t_parse('2 is 3 or 4 is 5', '(or (is 2 3) (is 4 5))')
    t_parse('e.x == 3 or e.x == 4', '(or (== (. e x) 3) (== (. e x) 4))')
    t_parse('if 1==2: 3\nelif 4:5\nelse:6', '(if (elif (== 1 2) 3) (elif 4 5) (else 6))')
    t_parse('x = y(2)*3 + y(4)*5', '(= x (+ (* ($ y 2) 3) (* ($ y 4) 5)))')
    t_parse('x(1,2)+y(3,4)', '(+ ($ x 1 2) ($ y 3 4))')
    t_parse('x(a,b,c[d])', '($ x a b (. c d))')
    t_parse('x(1,2)*j+y(3,4)*k+z(5,6)*l', '(+ (+ (* ($ x 1 2) j) (* ($ y 3 4) k)) (* ($ z 5 6) l))')
    t_parse('a = b.x/c * 2 - 1', '(= a (- (* (/ (. b x) c) 2) 1))')
    t_parse('for x in y: z', '(for x y z)')

    t_parse('min(255,n*2)','($ min 255 (* n 2))')
    t_parse('c = pal[i*8]','(= c (. pal (* i 8)))')
    t_parse('{x:y,a:b}','({} x y a b)')
    t_parse('x[1:2:3]','(. x (: 1 2 3))')
    if is_tinypy: t_parse('x - -234','(- x -234)')
    else: t_parse('x - -234','(- x -234.0)')
    t_parse('x - -y','(- x (- 0 y))')
    t_parse('x = ((y*4)-2)','(= x (- (* y 4) 2))')
    
    if is_tinypy: t_parse('print([1,2,"OK",4][-3:3][1])','($ print (. (. ([] 1 2 OK 4) (: -3 3)) 1))')
    else: t_parse('print([1,2,"OK",4][-3:3][1])','($ print (. (. ([] 1 2 OK 4) (: -3.0 3)) 1))')
    
    t_parse('a,b = 1,2','(= (, a b) (, 1 2))')
    t_parse('class C: pass','(class C (methods pass))')
    t_parse('def test(*v): pass','(def test ((): (* v)) pass)')
    t_parse('def test(**v): pass','(def test ((): (** v)) pass)')
    t_parse('test(*v)','($ test (* v))')
    t_parse('test(**v)','($ test (** v))')
    t_parse('def test(x=y): pass','(def test ((): (= x y)) pass)')
    t_parse('test(x=y)','($ test (= x y))')
    t_parse('def test(y="K",x="Z"): pass','(def test ((): (= y K) (= x Z)) pass)')
    t_parse('return x+y','(return (+ x y))')
    t_parse('if "a" is not "b": pass','(if (elif (isnot a b) pass))')
    t_parse('z = 0\nfor x in y: pass','(; (= z 0) (for x y pass))')
    t_parse('for k in {"OK":0}: pass','(for k ({} OK 0) pass)')
    t_parse('print(test(10,3,z=50000,*[200],**{"x":4000}))','($ print ($ test 10 3 (= z 50000) (* ([] 200)) (** ({} x 4000))))')
    t_parse('x="OK";print(x)','(; (= x OK) ($ print x))')
    t_parse('[(1,3)]','([] (, 1 3))')
    t_parse('x[:]','(. x (: None None))')
    t_parse('x[:1]','(. x (: None 1))')
    t_parse('x[1:]','(. x (: 1 None))')
    t_parse('return\nx','(; return x)')
    t_parse('"""test"""','test')
    t_parse('return a,b','(return (, a b))')
    
    

    # this should throw an error - bug #26
    t_unparse("""
while 1:
pass
""")
        
    # test cases for python 2.x print statements - bug #17
    # since this is python 2.x syntax it should output an syntax Exception
    
    # mini-block style
    t_unparse("""
def x(): print "OK"
""")

    # block level
    t_unparse("""
def x():
    print "OK"
""")

    # module level
    t_unparse("""
print "OK"
""")


        

################################################################################

def showerror(cmd, ss, ex, res):
    print(cmd)
    print("ss : '" + str(ss) + "'")
    print("ex : '" + str(ex) + "'")
    print("res: '" + str(res) + "'")

def t_render(ss,ex,exact=True):
    import tokenize, parse, encode
        
    if not istype(ss,'list'): ss =[ss]
    n = 1
    for s in ss:
        fname = 'tmp'+str(n)+'.tpc'
        system_rm(fname)
        tokens = tokenize.tokenize(s)
        t = parse.parse(s,tokens)
        r = encode.encode(fname,s,t)
        f = save(fname,r)
        n += 1
    system_rm('tmp.txt')
    cmd = VM + fname + " > tmp.txt"
    system(cmd)
    res = load(TMP).strip()
    
    #print(ss,ex,res)
    if exact:
        if res != ex: showerror(cmd, ss, ex, res)
        assert(res == ex)
    else: 
        if ex not in res: showerror(cmd, ss, ex, res)
        assert(ex in res)
        
def t_unrender(s):
    import tokenize, parse, encode
        
    ok = False
    try:
        tokens = tokenize.tokenize(s)
        t = parse.parse(s,tokens)
        r = encode.encode('tmp.tpc',s,t)
    except:
        ok = True
    assert(ok == True)

def test_range():
    t_render("""print(str(range(4))[:5])""","<list")
    t_render("""print(len(range(4)))""","4")
    t_render("""print(range(4)[0])""","0")
    t_render("""print(range(4)[1])""","1")
    t_render("""print(range(4)[-1])""","3")

    t_render("""print(str(range(-4))[:5])""","<list")
    t_render("""print(len(range(-4)))""","0")

    t_render("""print(str(range(0,5,3))[:5])""","<list")
    t_render("""print(len(range(0,5,3)))""","2")
    t_render("""print(range(0,5,3)[0])""","0")
    t_render("""print(range(0,5,3)[1])""","3")
    t_render("""print(range(0,5,3)[-1])""","3")

    t_render("""print(str(range(5,0,-3))[:5])""","<list")
    t_render("""print(len(range(5,0,-3)))""","2")
    t_render("""print(range(5,0,-3)[0])""","5")
    t_render("""print(range(5,0,-3)[1])""","2")
    t_render("""print(range(5,0,-3)[-1])""","2")

    t_render("""print(str(range(-8,-4))[:5])""","<list")
    t_render("""print(len(range(-8,-4)))""","4")
    t_render("""print(range(-8,-4)[0])""","-8")
    t_render("""print(range(-8,-4)[1])""","-7")
    t_render("""print(range(-8,-4)[-1])""","-5")

    t_render("""print(str(range(-4,-8,-1))[:5])""","<list")
    t_render("""print(len(range(-4,-8,-1)))""","4")
    t_render("""print(range(-4,-8,-1)[0])""","-4")
    t_render("""print(range(-4,-8,-1)[1])""","-5")
    t_render("""print(range(-4,-8,-1)[-1])""","-7")

    t_render("""print(str(range(-4,-8))[:5])""","<list")
    t_render("""print(len(range(-4,-8)))""","0")

    t_render("""print(str(range(-8,-4,-1))[:5])""","<list")
    t_render("""print(len(range(-8,-4,-1)))""","0")

    t_render("""print(str(range(0,4,0))[:5])""","<list")
    t_render("""print(len(range(0,4,0)))""","0")

    
if __name__ == '__main__':
    t_render('print("hello world")',"hello world")
    t_render('print(234)',"234")
    t_render('a=3\nprint(a)',"3")
    t_render('print(2+3)',"5")
    t_render('print(4|1)',"5")
    t_render('print(5&7)',"5")
    t_render('print(2^7)',"5")
    t_render('print(7^2&2)',"5")
    t_render('print(7^2|4)',"5")
    t_render('x=4\nx|=1\nprint(x)',"5")
    t_render('x=5\nx&=7\nprint(x)',"5")
    t_render('x=2\nx^=7\nprint(x)',"5")
    t_render("""
x = 2
x += 3
print(x)
"""
,"5")
    t_render("""
x = "OK"
print(x)
"""
,"OK")
    t_render("""
a,b = 1,2
print(a+b)
"""
,"3")
    t_render("""
x = 1
if x == 1:
    print("yes")
""","yes")
    t_render("""
if 0 == 1:
    print("X")
else:
    print("OK")
"""
,"OK")
    
    t_render("""
if 0 == 1:
    print("X")
elif 1 == 2:
    print("Y")
else:
    print("OK")
"""
,"OK")

    t_render("""
def test(x,y):
    return x+y
r = test(3,5)
print(r)
""","8")
    t_render("""
x = 1
t = 0
while x<=5:
    t = t+x
    x = x+1
print(t)
""","15")
    t_render("""
x = {}
x.y = "test"
print(x.y)
""","test")
    
    t_render("""
if "a" is "a":
    print("OK")
"""
,"OK")

    t_render("""
if "a" is not "b":
    print("OK")
"""
,"OK")

    t_render("""
if None is None:
    print("OK")
"""
,"OK")
    t_render("""
if "x" is "x" or "y" is "y":
    print("OK")
"""
,"OK")
    t_render("""
x = 1
while x < 3:
    break
    x = x + 1
print(x)
"""
,"1")
    t_render("""
x = 1
n = 0
while x < 10:
    x = x + 1
    if n == 2:
        continue
    n = n + 1
print(n)
"""
,"2")
    t_render("""
def test(x): return x
y = test(1)*2 + test(3)*4 + test(5)*6
print(y)
"""
,"44")
    t_render("""
def test(a,b): return a+b
print(test(1,1)+test(1,1))
"""
,"4")

    t_render("""
def test(): print("OK")
x = test
x()
"""
,"OK")

    t_render("""
x = [2,4,6]
print(x[1])
"""
,"4")
    t_render("""
def test(): print("OK")
x = [1,test,2]
x[1]()
"""
,"OK")


    t_render("""
z = 0
for x in [1,2,3]:
    z += x
print(z)
"""
,"6")

    t_render("""
z = 0
for x in range(1,4):
    z += x
print(z)
"""
,"6")

    t_render("""
x = {'a':'OK'}
print(x.a)
"""
,"OK")

    t_render("""print("1234"[1:3])""","23")
    t_render("""print("1234"[-3:3])""","23")
    t_render("""print([1,2,"OK",4][-3:3][1])""","OK")
    t_render("""
n = 0
for x in range(0,10,2):
    n += 1
print(n)
"""
,"5")

    t_render("""print(max(3,8,2,6))""","8")
    t_render("""print(min(3,4,2,6))""","2")
    t_render("""for k in {'OK':0}: print(k)""","OK")

    t_render("""
X = "OK"
def test(): print(X)
test()
"""
,"OK")

    t_render("""
a = 4
def test(z):
    for i in range(0,a):
        z += i
    return z
print(test(1))
"""
,"7")

    t_render("""
def test(self): print(self)
fnc = bind(test,"OK")
fnc()
"""
,"OK")


    t_render("""
x = [v*v for v in range(0,5)]
print(x[3])
"""
,"9")
    t_render("""
t = [[y*10+x for x in range(0,10)] for y in range(0,10)]
print(t[2][3])
"""
,"23")

    t_render("""
x = [1]
x.extend([2,3])
print(x[1])
"""
,"2")

    #t_render("""
#x = {'a':3}
#merge(x,{'b':4})
#print(x.b)
#"""
#,"4")

    t_render("""
x = [1,2,3]
y = copy(x)
y[0] *= 10
print(x[0]+y[0])
"""
,"11")
    t_render("""
x = {'a':3}
y = copy(x)
y.a *= 10
print(x.a+y.a)
"""
,"33")
    t_render("""
x = {}
y = x['x']
"""
,'KeyError', False)
    t_render("""
x = []
y = x[1]
"""
,'KeyError', False)
    t_render("""print("O"+"K")""","OK")
    t_render("""print("-".join(["O","K"]))""","O-K")
    t_render("""print("OK-OK".split("-")[1])""","OK")
    t_render("""
def test(*v): return max(v[2])
print(test(*[1,2,"OK"]))
"""
,"OK")
    t_render("""
def test(**v): return v['x']
print(test(**{'x':'OK'}))
"""
,"OK")
    #t_render("""
#def test(y='K',x='Z'): print(x+y)
#test(x='O')
#"""
#,"OK")
    t_render("""
def test(y='K',x='Z'): print(x+y)
test('O')
"""
,"ZO")

    #t_render("""
#def test(a,b=2,*c,**d): return a+b+c[0]+d['x']+d['z']
#print(test(10,3,z=50000,*[200],**{'x':4000}))
#"""
#,"54213")

    t_render("""print("".join(["O"]+["K"]))""","OK")
    t_render("""x="OK";print(x)""","OK")
    t_render("""x = [1,2,] ; print(x[1])""","2")
    t_render("""a,b,d = [0],0,'OK'; print(d)""","OK")
    
    t_render("""
def test():
    raise
try:
    test()
except:
    print("OK")
""","OK")

    t_render("""print("OKx"[:-1])""","OK")
    t_render("""print("xOK"[1:])""","OK")
    t_render("""a,b = "OK"; print(a+b)""","OK")
    
    t_render("""
def test(a,b):
    print (a+b[2])
test(1,3)
""","Exception",False)

    t_render("""
def test(): raise
test()
""","Exception",False)
    
    t_render(['OK="OK"',"import tmp1\nprint(tmp1.OK)"],"OK")
    
    t_render(['O="O"','K="K"',"import tmp1, tmp2\nprint(tmp1.O+tmp2.K)"],"OK")
        
    t_render("""
def test(): return
x = 1
print(test())
""","None")

    t_render("""
def test(): pass
x = 1
print(test())
""","None")

    t_render("""
def test():
    global x
    x = "OK"
test()
print(x)
""","OK")

    t_render("print(len([1,2,3]))","3")
    
    t_render('if not "?" in "xyz": print("OK")',"OK")
    
    t_render('print({1:"OK"}[1])',"OK")
    
    t_render('print(len("\0"))',"1")
    
    t_render('print(1 in {1:2})',"1")
    
    t_render('x = {1:2}; del x[1]; print(len(x))','0')
    
    t_render("""
def test(t):
    t = "O"+t
    print(t)
test("K")
""","OK")

    t_render("""print([1,2,3].index(3))""","2")
    t_render("""print("1,2,3".split(",").index("3"))""","2")
    
    t_render("""v = [3,2,1]; v.sort(); print(v[0])""","1")

    t_render("""print(abs(-5))""","5")
    t_render("""print(int(1.234))""","1")
    
    t_render("print(int(round(1.5)))","2")
    t_render("print(ord('X'))","88")
    t_render("print(ord(chr(128)))","128")
    #t_render("print(fsize('LICENSE.txt'))","181")
    t_render("print(int('ff',16))","255")
    t_render("""
def test(x,y): print(x); return y
test('a',1) or test('b',1) and test('c',0)
""","a")

    #t_render("def test(): print('OK')\n{'__call__':test}()","OK")


    t_render("""
def test():
    def fnc():
        print("OK")
    fnc()
test()
""","OK")

    t_render("""print("aa..bbb...ccc".replace("..","X"))""","aaXbbbX.ccc")
    t_render("""print("..bbb..".replace("..","X"))""","XbbbX")
    t_render("""print("234".replace("\r\n","\n"))""","234")
    t_render("""print("a\0b".replace("\0","X"))""","aXb")
    t_render("""x = "a\0b"; x = x.replace("\0","c"); print(x)""","acb")

    t_render("""print(0xff)""","255")
    
    t_render("""x=(1,3);print({x:'OK'}[x])""","OK")
    t_render("""x=(1,3);y=(1,3);print({x:'OK'}[y])""","OK")
    t_render("""print({(1,3):'OK'}[(1,3)])""","OK")
    t_render("def test(): test()\ntest()","Exception", False)
    t_render("x = []; x.append(x); print(x<x)","0");
    t_render("x = []; x.append(x); print({x:'OK'}[x])","OK")
    #t_render("print(float(str(4294967296))==float('4294967296'))","1")
    t_render("print(2**3)","8")
    #t_render("x = 'OK',\nprint(x[0])","OK")

    test_range()
    
    t_render(['v="OK"',"from tmp1 import *\nprint(v)"],"OK")
    t_render(['v="OK"',"from tmp1 import v\nprint(v)"],"OK")
    t_render(['x="X";y="K"',"x = 'O'\nfrom tmp1 import y\nprint(x+y)"],"OK")

    t_render("""
def test(**e):
    print(e['x'])
test(x='OK')
""","OK")

    # test register allocator
    s = "def f():pass\n"+("f()\n"*256)+"print('OK')"
    t_render(s,"OK")
    
    t_render("print(2**3)","8")
    t_render("print(2*3**2)", "18", False)
    
    
    t_render("""
def test(**v): return 'OK'
print(test())
"""
,"OK")
    t_render("""
def test(**v):
    v['x'] = 'OK'
    return v
print(test()['x'])
"""
,"OK")

    t_render("""
def get(self,k):
    return k+"K"
v = object()
v.__get__ = bind(get,v)
print(v.O)
""",
"OK")

    t_render("""
def set(self,k,v):
    self = getraw(self)
    self[k] = v + "K"
v = object()
v.__set__ = bind(set,v)
v.x = "O"
print(v.x)
""",
"OK")

    t_render("""
def call(self,x):
    print(x)
v = object()
v.__call__ = bind(call,v)
v("OK")
"""
,"OK")


    #a REG related test
    t_render("""
def test():
    init = True
    if init and True:
        pass
print("OK")
""","OK")

    meta_objs_init = """
def my_new(klass,*p):
    self = object()
    setmeta(self,klass)
    self.__init__(*p)
    return self

def A_init(self,v):
    if v: print("A_init")
def A_test1(self):
    print("A_test1")
def A_test2(self):
    print("A_test2")
A = {'__new__':my_new,'__init__':A_init,'test1':A_test1,'test2':A_test2}

def B_init(self,v):
    if v: print("B_init")
def B_test2(self):
    print("B_test2")
B = {'__init__':B_init,'test2':B_test2}
setmeta(B,A)
"""

    t_render(meta_objs_init+"""A(True)""","A_init")
    t_render(meta_objs_init+"""A(False).test1()""","A_test1")
    t_render(meta_objs_init+"""A(False).test2()""","A_test2")
    t_render(meta_objs_init+"""B(True)""","B_init")
    t_render(meta_objs_init+"""B(False).test1()""","A_test1")
    t_render(meta_objs_init+"""B(False).test2()""","B_test2")

    #various class construct use tests
    t_render("""
class C:
    def __init__(self,data): self.data = data
    def print(self): print(self.data)
C("OK").print()
"""
,"OK")

    t_render("""
class X:
    pass
y = X()
print("OK")
""","OK")

    t_render("""
class X: pass
def test(): y = X()
test()
print("OK")
""","OK")

    t_render(["class X: pass\ndef test(): y = X()","import tmp1\ntmp1.test();print('OK')"],"OK")

    t_render("""
class A:
    def __init__(self):
        self.a = 'O'
        self.b = 'x'
    def test(self):
        print("KO")
class B(A):
    def __init__(self):
        A.__init__(self)
        self.b = 'K'
    def test(self):
        print(self.a+self.b)
B().test()
""","OK")

    t_render("""
class A:
    def test(self):
        print(self)
A.test("OK")
""","OK")


    #test that you can make a callable object
    t_render("""
class Test:
    def __init__(self,v):
        self.value = v
    def __call__(self):
        print(self.value)

x = Test('OK')
x()
""","OK")

    #test that you can use a __get__
    t_render("""
class Test:
    def __get__(self,k):
        return k+"K"
x = Test()
print(x.O)
""","OK")
    #test that you can use __set__
    t_render("""
class Test:
    def __set__(self,k,v):
        getraw(self)[k] = "O"+v
x = Test()
x.v = "K"
print(x.v)
""","OK")

    #test that exceptions are cleared after they are caught
    #and not repeated
    t_render("""
def test():
    try:
        pass
    except:
        pass
    print("OK")
    raise
try:
    test()
except:
    pass
""","OK")

    #check that missing attributes throw an error
    t_render("""
class A: pass
try:
    A().x
except:
    print('OK')
""","OK")

    #check that a changed attribute gets changed
    t_render("""
class A:
    def x(self): pass
a = A()
a.x = "OK"
print(a.x)
""","OK")
    
    #test that you can use a __get__ gets inherited
    t_render("""
class A:
    def __get__(self,k):
        return k+"K"
class B(A):
    pass
x = B()
print(x.O)
""","OK")

    #test that meta methods aren't called on non-objects
    t_render("""
def get(): pass
x = {"__get__":get}
try:
    z = x.y
except:
    print("OK")
""","OK")

    #test that meta stuff is inheritited in dicts
    t_render("""
x = {1:"O"}
y = {2:"K"}
setmeta(y,x)
print(y[1]+y[2])
""","OK")

    #test that meta stuff doesn't change into methods in dicts
    t_render("""
def get(k): return k
x = {"get":get}
print(x.get("OK"))
""","OK")

    #tests issue 14: string.index() should give an exception if substring not found
    t_render("""
try:
    "test-".index("=")
except:
    print("OK")
""","OK")

    #issue 19: test that multiplying a string with a negative value returns an empty string
    t_render("""
foo = "abc" * -1
print(foo)
""", "")

    #issue 18:  tests that strings containing NUL chars are printed correctly
    t_render("""
foo = "abc" + chr(0) + "d"
print(foo)
""", "abc" + chr(0) + "d")

    #issue 18 (related): tests that "".strip() treats strings containing NUL chars correctly
    t_render("""
foo = "abc" + chr(0) + "d\n"
print(foo.strip())
""", "abc" + chr(0) + "d")

    #test that class variables work as expected
    t_render("""
class A:
    foo = 42
s = str(A.foo)
A.foo += 1
s += str(A.foo)
print(s)
""", "4243")

    #check that class variables are not leaked to the global scope
    t_render("""
class A:
    foo = 42
print(foo)
""", "KeyError", False)

    #test that class variables can correctly be accessed by a subclass
    t_render("""
class A:
    foo = "OK"
class B(A):
    pass
print(B.foo)
""", "OK")

    #test that class variables can be accessed from instances
    t_render("""
class A:
    foo = "OK"
o = A()
print(o.foo)
""", "OK")

    #test case for possible register allocation bug #22
    t_render("""
x = [1, 2, 3]

for i in x:
    if i != 0 and i:
        y = "OK"
print(y)
""","OK")

    #test that sandbox() raises an exception when the time limit is passed
    if SANDBOX:
        t_render("""
sandbox(1, False)
while True: 
    pass
""", "SandboxError", False)

    #test that calling sandbox() removes the sandbox builtin
    if SANDBOX:
        t_render("""
sandbox(500, False)
try:
    sandbox(200)
except:
    print("OK")
""", "OK")

    #test that sandbox() raises an exception when the memory limit is passed
    if SANDBOX:
        t_render("""
sandbox(False, 1)
a = 42
""", "SandboxError", False)

    #test that circular inheritance doesn't cause an infinite lookup chain
    t_render("""
class A:
    pass

class B:
    pass

setmeta(A, B)
setmeta(B, A)

foo = A()
print("OK")
""", "tp_lookup",False)


    #tests issue #20: test that string multiplication is commutative
    t_render("""
foo = "O"
bar = "K"
print(3 * foo, bar * 3)
""", "OOO KKK")

    #test issue #27: that the __main__ module doesn't get GC'd
    t_render("""
MODULES["__main__"] = None
for n in range(0,50000):
    x = [n]
print("OK")
""","OK")

    #test case for UnboundLocalError - bug #16
    t_unrender("""
def foo():
    print(v)
    v = "ERROR"
""")

    #test for segfault on large split
    t_render("""
x = " ".join([str(n) for n in range(0,50000)])
y = x.split("1")
print("OK")
""","OK")

    #test for Issue 42: 'not' operator only works for numbers, 
    #not dicts, lists, or strings
    #Reported by kiwidrew, Apr 08, 2009
    #see also: http://code.google.com/p/tinypy/issues/detail?id=42
    t_render("""
if not None:
    print('OK')
""", "OK")

    t_render("""
n = 0
if not n:
    print('OK')
""","OK")

    t_render("""
d = {}
if not d:
    print('OK')
""","OK")

    t_render("""
l = []
if not l:
    print('OK')
""","OK")

    t_render("""
s = ''
if not s:
    print('OK')
""","OK")

################################################################################

def t_boot(ss,ex,exact=True):
    if not istype(ss,'list'): ss =[ss]
    n = 1
    for s in ss:
        fname = 'tmp'+str(n)+'.tpc'
        system_rm(fname)
        fname = 'tmp'+str(n)+'.py'
        save(fname,s)
        n += 1
    system_rm('tmp.txt')
    #system(TINYPY+fname+' > tmp.txt')
    system("../build/tinypy "+fname+' > tmp.txt')
    res = load(TMP).strip()
    print(ss,ex,res)
    if exact: assert(res == ex)
    else: assert(ex in res)

is_boot = False
try:
    assert(is_tinypy == True)
    x = compile('x=3','')
    is_boot = True
except:
    pass

def t_api(ss_py,ss,ex):
    if not '-linux' in ARGV:
        return
    
    #first verify that the python code produces the result
    t_render(ss_py,ex) 
    
    #then verify that the C code does ...
    fname = "tmp.c"
    system_rm("tmp.c")
    system_rm("tmp")
    system_rm(TMP)
    save(fname,ss)
    system("gcc tmp.c -Wall -g -lm -o tmp")
    cmd = "./tmp > "+TMP
    system(cmd)
    res = load(TMP).strip()
    if res != ex: showerror(cmd, ss, ex, res)
    assert(res == ex)

if is_boot == True and __name__ == '__main__':
    print("# t_boot")
    t_boot(["def test(): print('OK')","import tmp1; tmp1.test()"],"OK")

    print("# t_api")
    
    # all t_api examples include a python equivalent
    # also include a brief explanation of the point
    # of the example
    
    # just a "hello world" style example
    t_api("""
print ("OK")
""","""
#include "tp.c"
int main(int argc, char *argv[]) {
    tp_vm *tp = tp_init(argc,argv);
    
    tp_params_v(tp,1,tp_string("OK"));
    tp_print(tp);
    
    tp_deinit(tp);
    return(0);
}
""",
"OK")

    # how to create a c function and call it
    t_api("""
def test(v):
    print(v)
test("OK")
""","""
#include "tp.c"
tp_obj test(TP) {
    tp_obj v = TP_OBJ();
    tp_params_v(tp,1,v);
    tp_print(tp);
    return tp_None;
}
int main(int argc, char *argv[]) {
    tp_vm *tp = tp_init(argc,argv);
    
    tp_obj fnc = tp_fnc(tp,test);
    tp_call(tp,fnc,tp_params_v(tp,1,tp_string("OK")));
    
    tp_deinit(tp);
    return(0);
}
""",
"OK")

    # how to create a simple class
    t_api("""
class A:
    def __init__(self,value):
        self.value = value
    def test(self):
        print(self.value)
A("OK").test()
""","""

#include "tp.c"
tp_obj A_init(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj v = TP_OBJ();
    tp_set(tp,self,tp_string("value"),v);
    return tp_None;
}
tp_obj A_test(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj v = tp_get(tp,self,tp_string("value"));
    tp_params_v(tp,1,v);
    tp_print(tp);
    return tp_None;
}
int main(int argc, char *argv[]) {
    tp_vm *tp = tp_init(argc,argv);
    
    /* create our class */
    tp_obj tmp;
    tp_obj A = tp_class(tp);
    tp_set(tp,A,tp_string("__init__"),tp_fnc(tp,A_init));
    tp_set(tp,A,tp_string("test"),tp_fnc(tp,A_test));
    
    /* instantiate it and call test */
    tmp = tp_call(tp,A,tp_params_v(tp,1,tp_string("OK")));
    tp_call(tp,tp_get(tp,tmp,tp_string("test")),tp_params_v(tp,0));
    
    tp_deinit(tp);
    return(0);
}
""",
"OK")

################################################################################

def unformat(x):
    x = x.split(' ')
    r = []
    for i in x:
        if i != ':':
            if i:
                r.append(i)
    return " ".join(r)

def t_asm(ass, ex, exact=True,check_dis=True):
    ass = ass.strip()
    bc = asm.assemble(ass)
    dis = disasm.disassemble(bc)
    dis = unformat(dis)
    if check_dis and dis != ass:
        print (ass)
        print (dis)
        assert(dis == ass)
    fname = "tmp.tpc"
    system_rm(fname)
    system_rm(TMP)
    save(fname,bc)
    cmd = VM + fname + " > " + TMP
    system(cmd)
    res = load("tmp.txt").strip()
    if exact:
        if ex != res:
            print (ass)
            print (ex)
            print (res)
            assert(ex == res)
    else:
        if ex not in res:
            print (ass)
            print (ex)
            print (res)
            assert(ex in res)
    
if is_boot == True and __name__ == '__main__':
    print("# t_asm")
    
    t_asm("""
NUMBER 0 0 0 42
DEBUG 0 0 0
EOF 0 0 0
""", "DEBUG: 0 42")

    t_asm("""
STRING 0 0 1 "a"
NUMBER 1 0 0 41
GSET 0 1 0
STRING 2 0 1 "a"
GGET 1 2 0
NUMBER 2 0 0 1
ADD 1 1 2
GSET 0 1 0
STRING 2 0 5 "print"
GGET 1 2 0
STRING 3 0 1 "a"
GGET 2 3 0
PARAMS 0 2 1
CALL 0 1 0
EOF 0 0 0
""", "42")

    t_asm("""
STRING 0 0 4 "str1"
STRING 1 0 3 "foo"
GSET 0 1 0
STRING 0 0 4 "str2"
STRING 1 0 3 "bar"
GSET 0 1 0
STRING 2 0 5 "print"
GGET 1 2 0
STRING 3 0 4 "str1"
GGET 2 3 0
STRING 4 0 4 "str2"
GGET 3 4 0
ADD 2 2 3
PARAMS 0 2 1
CALL 0 1 0
EOF 0 0 0
""", "foobar")

    t_asm("""
STRING 0 0 3 "foo"
STRING 1 0 3 "bar"
NUMBER 2 0 0 1
IF 2 0 0
ADD 0 0 1
STRING 1 0 5 "print"
GGET 3 1 0
PARAMS 2 0 1
CALL 0 3 2
EOF 0 0 0
""", "foo");

    t_asm("""
NUMBER 0 0 0 1
NUMBER 1 0 0 2
JUMP 0 0 2
ADD 0 0 1
DEBUG 0 0 0
EOF 0 0 0
""", "DEBUG: 0 1");

    t_asm("""
STRING 0 0 3 "foo"
NUMBER 1 0 0 3
MUL 0 0 1
DEBUG 0 0 0
EOF 0 0 0
""", "DEBUG: 0 foofoofoo");

    t_asm("""
NUMBER 0 0 0 1
NUMBER 0 0 0 42
LIST 0 0 2
GET 0 0 1
DEBUG 0 0 0
EOF 0 0 0
""", "DEBUG: 0 42");

    t_asm("""
NUMBER 0 0 0 1
NUMBER 1 0 0 1
NUMBER 2 0 0 1
LIST 0 0 3
LEN 0 0 0
DEBUG 0 0 0
EOF 0 0 0
""", "DEBUG: 0 3");

    t_asm("""
DEF 0 0 13
STRING 1 0 3 "foo"
NAME 1 0 0
STRING 3 0 5 "print"
GGET 2 3 0
STRING 3 0 3 "foo"
PARAMS 1 3 1
CALL 1 2 1
EOF 0 0 0
PARAMS 1 0 0
CALL 1 0 1
EOF 0 0 0
""", "foo");

    #test that function definitions longer than the bytecode are properly sanitized
    if SANDBOX:
        t_asm("""
DEF 0 0 100
REGS 2 0 0
STRING 1 0 3 "foo"
NAME 1 0 0
PASS 0 0 0
EOF 0 0 0
""", "SandboxError", False)

    #test that negative out of bounds jumps are sanitized
    if SANDBOX:
        t_asm("""
JUMP 0 127 255
EOF 0 0 0
""", "SandboxError", False)
    
    #test that positive out of bounds jumps are sanitized
    if SANDBOX:
        t_asm("""
JUMP 0 128 0
EOF 0 0 0
""", "SandboxError", False)

    #test that strings with boundaries beyond the end of the bytecode are properly sanitized
    if SANDBOX:
        t_asm("""
STRING 1 0 100 "foobar"
EOF 0 0 0
""", "SandboxError", False,False)

