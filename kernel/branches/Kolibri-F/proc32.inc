
; Macroinstructions for defining and calling procedures

; @brief Directly call STDCALL procedure
; @param proc Callee name
; @param arg Arguments to pass
macro stdcall proc,[arg]
 { common
    if ~ arg eq
   reverse
        pushd   arg
   common
    end if
        call    proc }

; @brief Indirectly call STDCALL procedure
; @param proc Callee name
; @param arg Arguments to pass
macro invoke proc,[arg]
 { common
    if ~ arg eq
   reverse
        pushd   arg
   common
    end if
        call    [proc] }

; @brief Directly call CDECL procedure
; @param proc Callee name
; @param arg Arguments to pass
macro ccall proc,[arg]
 { common
    size@ccall = 0
    if ~ arg eq
   reverse
        pushd   arg
    size@ccall = size@ccall+4
   common
    end if
        call    proc
    if size@ccall
        add     esp, size@ccall
    end if }

; @brief Indirectly call CDECL procedure
; @param proc Callee name
; @param arg Arguments to pass
macro cinvoke proc,[arg]
 { common
    size@ccall = 0
    if ~ arg eq
   reverse
        pushd   arg
    size@ccall = size@ccall+4
   common
    end if
        call    [proc]
    if size@ccall
        add     esp, size@ccall
    end if }

; @brief Define a procedure.\n
;        Calling convention for the procedure may be defined before parameter
;        list using `stdcall` or `c` word like this:\n
;        `proc name stdcall, param0, param1`\n
;        List of registers used in the procedure may be specified before
;        parameter list using `uses` word like this:\n
;        `proc name uses eax ebx ecx, param0, param1`\n
;        If you need to specify both calling convention and used registers
;        put calling convention first and then `uses` statement like this:\n
;        `proc name stdcall uses ebx ecx edx, param0, param1`\n
;        The defined procedure should be ended using `endp` macro.
; @param args Name of the procedure and a comma-separated argument list.
;             Type of any parameter may be specified by semicolon after its
;             name like this:\n
;             `proc name param0:dword, param1:qword`.
macro proc [args]
 { common
    match name params, args>
    \{ define@proc name,<params \} }

prologue@proc equ prologuedef

; @dont_give_a_doxygen
macro prologuedef procname,flag,parmbytes,localbytes,reglist
 { local loc
   loc = (localbytes+3) and (not 3)
   parmbase@proc equ ebp+8
   localbase@proc equ ebp-loc
   if parmbytes | localbytes
        push    ebp
        mov     ebp, esp
    if localbytes
        sub     esp, loc
    end if
   end if
   irps reg, reglist \{ push reg \} }

epilogue@proc equ epiloguedef

; @dont_give_a_doxygen
macro epiloguedef procname,flag,parmbytes,localbytes,reglist
 { irps reg, reglist \{ reverse pop reg \}
   if parmbytes | localbytes
        leave
   end if
   if flag and 10000b
        retn
   else
        retn    parmbytes
   end if }

close@proc equ

; @dont_give_a_doxygen
macro define@proc name,statement
 { local params,flag,regs,parmbytes,localbytes,current
   if used name
   name:
   match =stdcall args, statement \{ params equ args
                                     flag = 11b \}
   match =stdcall, statement \{ params equ
                                flag = 11b \}
   match =c args, statement \{ params equ args
                               flag = 10001b \}
   match =c, statement \{ params equ
                          flag = 10001b \}
   match =params, params \{ params equ statement
                            flag = 0 \}
   match =uses reglist=,args, params \{ regs equ reglist
                                        params equ args \}
   match =regs =uses reglist, regs params \{ regs equ reglist
                                             params equ \}
   match =regs, regs \{ regs equ \}
   match prologue:reglist, prologue@proc:<regs> \{ prologue name,flag,parmbytes,localbytes,reglist \}
   virtual at parmbase@proc
   match =,args, params \{ defargs@proc args \}
   match =args@proc args, args@proc params \{ defargs@proc args \}
   parmbytes = $-(parmbase@proc)
   end virtual
   name # % = parmbytes/4
   all@vars equ
   current = 0
   macro locals
   \{ virtual at localbase@proc+current
      macro label def \\{ match . type,def> \\\{ deflocal@proc .,label,<type \\\} \\}
      struc db [val] \\{ \common deflocal@proc .,db,val \\}
      struc du [val] \\{ \common deflocal@proc .,du,val \\}
      struc dw [val] \\{ \common deflocal@proc .,dw,val \\}
      struc dp [val] \\{ \common deflocal@proc .,dp,val \\}
      struc dd [val] \\{ \common deflocal@proc .,dd,val \\}
      struc dt [val] \\{ \common deflocal@proc .,dt,val \\}
      struc dq [val] \\{ \common deflocal@proc .,dq,val \\}
      struc rb cnt \\{ deflocal@proc .,rb cnt, \\}
      struc rw cnt \\{ deflocal@proc .,rw cnt, \\}
      struc rp cnt \\{ deflocal@proc .,rp cnt, \\}
      struc rd cnt \\{ deflocal@proc .,rd cnt, \\}
      struc rt cnt \\{ deflocal@proc .,rt cnt, \\}
      struc rq cnt \\{ deflocal@proc .,rq cnt, \\} \}
   macro endl
   \{ purge label
      restruc db,du,dw,dp,dd,dt,dq
      restruc rb,rw,rp,rd,rt,rq
      current = $-(localbase@proc)
      end virtual \}
   macro ret operand
   \{ match any, operand \\{ retn operand \\}
      match , operand \\{ match epilogue:reglist, epilogue@proc:<regs> \\\{ epilogue name,flag,parmbytes,localbytes,reglist \\\} \\} \}
   macro finish@proc
   \{ localbytes = current
      match close:reglist, close@proc:<regs> \\{ close name,flag,parmbytes,localbytes,reglist \\}
      end if \} }

; @dont_give_a_doxygen
macro defargs@proc [arg]
 { common
    if ~ arg eq
   forward
     local ..arg,current@arg
     match argname:type, arg
      \{ current@arg equ argname
         label ..arg type
         argname equ ..arg
         if dqword eq type
           dd ?,?,?,?
         else if tbyte eq type
           dd ?,?,?
         else if qword eq type | pword eq type
           dd ?,?
         else
           dd ?
         end if \}
     match =current@arg,current@arg
      \{ current@arg equ arg
         arg equ ..arg
         ..arg dd ? \}
   common
     args@proc equ current@arg
   forward
     restore current@arg
   common
    end if }

; @dont_give_a_doxygen
macro deflocal@proc name,def,[val] { name def val }

; @dont_give_a_doxygen
macro deflocal@proc name,def,[val]
 { common
    match vars, all@vars \{ all@vars equ all@vars, \}
    all@vars equ all@vars name
   forward
    local ..var,..tmp
    ..var def val
    match =?, val \{ ..tmp equ \}
    match any =?, val \{ ..tmp equ \}
    match any (=?), val \{ ..tmp equ \}
    match =label, def \{ ..tmp equ \}
    match tmp : value, ..tmp : val
     \{ tmp: end virtual
        initlocal@proc ..var,def value
        virtual at tmp\}
   common
    match first rest, ..var, \{ name equ first \} }

struc label type { label . type }

; @dont_give_a_doxygen
macro initlocal@proc name,def
 { virtual at name
    def
    size@initlocal = $ - name
   end virtual
   position@initlocal = 0
   while size@initlocal > position@initlocal
    virtual at name
     def
     if size@initlocal - position@initlocal < 2
      current@initlocal = 1
      load byte@initlocal byte from name+position@initlocal
     else if size@initlocal - position@initlocal < 4
      current@initlocal = 2
      load word@initlocal word from name+position@initlocal
     else
      current@initlocal = 4
      load dword@initlocal dword from name+position@initlocal
     end if
    end virtual
    if current@initlocal = 1
        mov     byte [name+position@initlocal], byte@initlocal
    else if current@initlocal = 2
        mov     word [name+position@initlocal], word@initlocal
    else
        mov     dword [name+position@initlocal], dword@initlocal
    end if
    position@initlocal = position@initlocal + current@initlocal
   end while }

; @brief Mark the end of a procedure created by `proc` macro
macro endp
 { purge ret,locals,endl
   finish@proc
   purge finish@proc
   restore regs@proc
   match all,args@proc \{ restore all \}
   restore args@proc
   match all,all@vars \{ restore all \} }

macro local [var]
 { common
    locals
   forward done@local equ
    match varname[count]:vartype, var
    \{ match =BYTE, vartype \\{ varname rb count
                                restore done@local \\}
       match =WORD, vartype \\{ varname rw count
                                restore done@local \\}
       match =DWORD, vartype \\{ varname rd count
                                 restore done@local \\}
       match =PWORD, vartype \\{ varname rp count
                                 restore done@local \\}
       match =QWORD, vartype \\{ varname rq count
                                 restore done@local \\}
       match =TBYTE, vartype \\{ varname rt count
                                 restore done@local \\}
       match =DQWORD, vartype \\{ label varname dqword
                                  rq count+count
                                  restore done@local \\}
       match , done@local \\{ virtual
                               varname vartype
                              end virtual
                              rb count*sizeof.\#vartype
                              restore done@local \\} \}
    match :varname:vartype, done@local:var
    \{ match =BYTE, vartype \\{ varname db ?
                                restore done@local \\}
       match =WORD, vartype \\{ varname dw ?
                                restore done@local \\}
       match =DWORD, vartype \\{ varname dd ?
                                 restore done@local \\}
       match =PWORD, vartype \\{ varname dp ?
                                 restore done@local \\}
       match =QWORD, vartype \\{ varname dq ?
                                 restore done@local \\}
       match =TBYTE, vartype \\{ varname dt ?
                                 restore done@local \\}
       match =DQWORD, vartype \\{ label varname dqword
                                  dq ?,?
                                  restore done@local \\}
       match , done@local \\{ varname vartype
                              restore done@local \\} \}
    match ,done@local
    \{ var
       restore done@local \}
   common
    endl }
