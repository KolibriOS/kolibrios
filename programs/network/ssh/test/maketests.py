file = open('numtests.txt', mode = 'r', encoding = 'utf-8-sig')
lines = file.readlines()
file.close()
commands = {"Sum", "Product", "LShift", "LShift1", "ModExp", "Remainder"}
command = ""
result = 0
A = 0
E = 0
B = 0
N = 0
M = 0
ctr = 0

def print_mpint(a, A):
    print(a + " dd " + str((A.bit_length() + 7) // 8))
    print("          db ", end='')
    for byte in A.to_bytes(((A.bit_length() + 7) // 8),"little"):
        print(hex(byte) + ", ", end='')
    print("0x0")
    print("          rb MPINT_MAX_LEN - " + str(max((((A.bit_length() + 7) // 8) + 1), 0)))    
    

for line in lines:
    words = line.split()
    if line[0] == '#':
        print(";" + line[1:], end='')
    elif words[0] in commands:
        command = words[0] 
        result = int(words[2], 16)
        ctr+=1
    elif words[0] == 'A':
        A = int(words[2], 16)
        if (command == 'LShift1'):
            if (A >= 0):
                print("stdcall mpint_shl1, mpint_A" + str(ctr))
                print("stdcall mpint_cmp, mpint_A" + str(ctr) + ", mpint_result" + str(ctr))
                print("je @f")
                print("mov eax, " + str(ctr))                 
                print("int3")
                print("@@:")
                print("iglobal")
                print_mpint(str("mpint_A" + str(ctr)), A)
                print_mpint(str("mpint_result" + str(ctr)), result)                 
                print("endg")
                print()           
    elif words[0] == 'B':
        B = int(words[2], 16)
        if (command == 'Sum'):
            if (A >= 0) & (B >= 0):
                print("stdcall mpint_add, mpint_B" + str(ctr) + ", mpint_A" + str(ctr))
                print("stdcall mpint_cmp, mpint_B" + str(ctr) + ", mpint_result" + str(ctr))
                print("je @f")
                print("mov eax, " + str(ctr))                 
                print("int3")
                print("@@:")
                print("iglobal")
                print_mpint(str("mpint_A" + str(ctr)), A)
                print_mpint(str("mpint_B" + str(ctr)), B)
                print_mpint(str("mpint_result" + str(ctr)), result)                 
                print("endg")
                print()
            elif (A <= 0) & (B >= 0) & (result >= 0): 
                print("stdcall mpint_sub, mpint_B" + str(ctr) + ", mpint_A" + str(ctr))
                print("stdcall mpint_cmp, mpint_B" + str(ctr) + ", mpint_result" + str(ctr))
                print("je @f")
                print("mov eax, " + str(ctr))                 
                print("int3")
                print("@@:")
                print("iglobal")
                print_mpint(str("mpint_A" + str(ctr)), -A)
                print_mpint(str("mpint_B" + str(ctr)), B)
                print_mpint(str("mpint_result" + str(ctr)), result)                 
                print("endg")
                print()
            elif (A >= 0) & (B <= 0) & (result >= 0): 
                print("stdcall mpint_sub, mpint_A" + str(ctr) + ", mpint_B" + str(ctr))
                print("stdcall mpint_cmp, mpint_A" + str(ctr) + ", mpint_result" + str(ctr))
                print("je @f")
                print("mov eax, " + str(ctr))                 
                print("int3")
                print("@@:")
                print("iglobal")
                print_mpint(str("mpint_A" + str(ctr)), A)
                print_mpint(str("mpint_B" + str(ctr)), -B)
                print_mpint(str("mpint_result" + str(ctr)), result)                 
                print("endg")
                print()                   
        if (command == 'Product'):
            if (A >= 0) & (B >= 0):
                print("stdcall mpint_mul, mpint_tmp, mpint_B" + str(ctr) + ", mpint_A" + str(ctr))
                print("stdcall mpint_cmp, mpint_tmp, mpint_result" + str(ctr))
                print("je @f")
                print("mov eax, " + str(ctr))                
                print("int3")
                print("@@:")
                print("iglobal")
                print_mpint(str("mpint_A" + str(ctr)), A)
                print_mpint(str("mpint_B" + str(ctr)), B)
                print_mpint(str("mpint_result" + str(ctr)), result)                 
                print("endg")
                print()
        if (command == 'Remainder'):
            if (A >= 0) & (B >= 0):
                print("stdcall mpint_mod, mpint_A" + str(ctr) + ", mpint_B" + str(ctr))
                print("stdcall mpint_cmp, mpint_A" + str(ctr) + ", mpint_result" + str(ctr))
                print("je @f")
                print("mov eax, " + str(ctr))                 
                print("int3")
                print("@@:")
                print("iglobal")
                print_mpint(str("mpint_A" + str(ctr)), A)
                print_mpint(str("mpint_B" + str(ctr)), B)
                print_mpint(str("mpint_result" + str(ctr)), result)                 
                print("endg")
                print()                  
    elif words[0] == 'N':
        N = int(words[2], 16)
        if (command == 'LShift'):
            if (A >= 0):
                print("stdcall mpint_shlmov, mpint_tmp, mpint_A" + str(ctr) + ", " + str(N))
                print("stdcall mpint_cmp, mpint_tmp, mpint_result" + str(ctr))
                print("je @f")
                print("mov eax, " + str(ctr))                 
                print("int3")
                print("@@:")                
                print("stdcall mpint_shl, mpint_A" + str(ctr) + ", " + str(N))
                print("stdcall mpint_cmp, mpint_A" + str(ctr) + ", mpint_result" + str(ctr))
                print("je @f")
                print("mov eax, " + str(ctr))                 
                print("int3")
                print("@@:")
                print("iglobal")
                print_mpint(str("mpint_A" + str(ctr)), A)
                print_mpint(str("mpint_result" + str(ctr)), result)                 
                print("endg")
                print()
    elif words[0] == 'E':
        E = int(words[2], 16)
    elif words[0] == 'M':
        M = int(words[2], 16)
        if (command == 'ModExp'):
            if (A >= 0) & (B >= 0):
                print("stdcall mpint_modexp, mpint_tmp, mpint_A" + str(ctr) + ", mpint_E" + str(ctr) + ", mpint_M" + str(ctr))
                print("stdcall mpint_cmp, mpint_tmp, mpint_result" + str(ctr))
                print("je @f")
                print("mov eax, " + str(ctr))                 
                print("int3")
                print("@@:")
                print("iglobal")
                print_mpint(str("mpint_A" + str(ctr)), A)
                print_mpint(str("mpint_E" + str(ctr)), E)
                print_mpint(str("mpint_M" + str(ctr)), M)                
                print_mpint(str("mpint_result" + str(ctr)), result)                 
                print("endg")
                print()          
    else:
        command = ''
