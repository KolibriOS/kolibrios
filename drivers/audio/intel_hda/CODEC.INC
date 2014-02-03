
;; Compose a 32bit command word to be sent to the HD-audio controller
proc make_codec_cmd stdcall, nid:dword, direct:dword, verb:dword, parm:dword
    push   ebx

    and    dword [codec.addr], 0xF
    and    dword [direct], 1
    and    dword [nid], 0x7F
    and    dword [verb], 0xFFF
    and    dword [parm], 0xFFFF

    mov    eax, [codec.addr]
    shl    eax, 28
    mov    ebx, [direct]
    shl    ebx, 27
    or     eax, ebx
    mov    ebx, [nid]
    shl    ebx, 20
    or     eax, ebx
    mov    ebx, [verb]
    shl    ebx, 8
    or     eax, ebx
    mov    ebx, [parm]
    or     eax, ebx
    pop    ebx
    ret
 .err:
    pop    ebx
    mov    eax, -1
    ret
endp

;; Send and receive a verb
proc  codec_exec_verb stdcall, cmd:dword;, res:dword <- returned in eax
    push   ebx  edx
    mov    ebx, [cmd]
    cmp    ebx, -1
    jne    @f
    pop    edx  ebx
    mov    eax, -1
    ret
  @@:
  if  FDEBUG   ;YAHOO
    push   eax esi
    mov    esi, msgVerbQuery
    call   SysMsgBoardStr
    mov    eax, ebx
    stdcall fdword2str, 2
    call   SysMsgBoardStr
    pop    esi eax
  end if

    mov    edx, -1
  .again:
    ; call   snd_hda_power_up
    stdcall  azx_send_cmd, ebx
    mov    ebx, eax
    test   ebx, ebx
    jnz    @f
    call  azx_get_response
    mov    edx, eax
  if  FDEBUG
    test   edx, edx
    jz     .end_debug
    push   eax esi
    mov    esi, msgVerbAnswer
    call   SysMsgBoardStr
    mov    eax, edx
    stdcall fdword2str, 2
    call   SysMsgBoardStr
    pop    esi eax
   .end_debug:
  end if

  @@:

    ;call   snd_hda_power_down
    cmp    edx, -1
    jne    .l1

    mov    eax, [ctrl.rirb_error]
    test   eax, eax
    jz     .l1

    mov    eax, [ctrl.response_reset]
    jz     @f

  if  DEBUG
    push   esi
    mov    esi, emsgBusResetFatalComm
    call   SysMsgBoardStr
    pop    esi
  end if
    call   azx_bus_reset
  @@:
  .l1:
    ;; clear reset-flag when the communication gets recovered
    test   ebx, ebx
    jnz    @f
    mov    [ctrl.response_reset], 0
  @@:
    mov    eax, edx

    pop    edx  ebx
    ret
endp


;;
;; snd_hda_codec_read - send a command and get the response
;; @nid: NID to send the command
;; @direct: direct flag
;; @verb: the verb to send
;; @parm: the parameter for the verb
;;
;; Send a single command and read the corresponding response.
;;
;; Returns the obtained response value, or -1 for an error.
;;
proc  snd_hda_codec_read stdcall, nid:dword, direct:dword, verb:dword, parm:dword
    stdcall  make_codec_cmd, [nid], [direct], [verb], [parm]
    stdcall  codec_exec_verb, eax
    ret
endp


;;
;; snd_hda_codec_write - send a single command without waiting for response
;; @nid: NID to send the command
;; @direct: direct flag
;; @verb: the verb to send
;; @parm: the parameter for the verb
;;
;; Send a single command without waiting for response.
;;
;; Returns 0 if successful, or a negative error code.
;;
proc  snd_hda_codec_write stdcall, nid:dword, direct:dword, verb:dword, parm:dword
    ; Do we need to support a sync write?
    stdcall  make_codec_cmd, [nid], [direct], [verb], [parm]
    stdcall  codec_exec_verb, eax
    ret
endp


;;
;; snd_hda_sequence_write - sequence writes
;; @seq: VERB array to send
;;
;; Send the commands sequentially from the given array.
;; The array must be terminated with NID=0.
;;
proc  snd_hda_sequence_write stdcall, seq:dword
    push     eax  ebx  ecx  esi
    mov      esi, [seq]
  @@:
    ;mov      ecx, [esi + hda_verb.nid]
    ;mov      ebx, [esi + hda_verb.verb]
    ;mov      eax, [esi + hda_verb.param]
    ;stdcall  snd_hda_codec_write, ecx, 0, ebx, eax
    ;add      esi, hda_verb.sizeof
    ;test     ecx, ecx
    ;jnz      @b
    ;______________________________________
    cmp      dword [esi], 0
    je       .out
    movzx    ecx, word [esi] ; NID
    movzx    ebx, word [esi+2] ; verb
    and      bx, 0x0FFF
    movzx    eax, word [esi + 4] ; sizeof(param) = 4 bytes
    stdcall  snd_hda_codec_write, ecx, 0, ebx, eax
    add      esi, 6
    jmp      @b
 .out:
    pop      esi  ecx  ebx  eax
    ret
endp


macro  snd_hda_param_read  nid, param
{
    stdcall  snd_hda_codec_read, nid, 0, AC_VERB_PARAMETERS, param
}

;;
;; snd_hda_get_sub_nodes - get the range of sub nodes
;; @codec: the HDA codec
;; @nid: NID to parse
;; @start_id: the pointer to store the start NID
;;
;; Parse the NID and store the start NID of its sub-nodes.
;; Returns the number of sub-nodes.
;;
proc  snd_hda_get_sub_nodes stdcall, nid:dword;, start_id:dword  <- returned in upper word of eax
    snd_hda_param_read  [nid], AC_PAR_NODE_COUNT

    cmp      eax, -1
    jne      @f
    inc      eax
  @@:
    and      eax, 0x7FFF7FFF

    ret
endp

;;
;; snd_hda_get_connections - get connection list
;; @codec: the HDA codec
;; @nid: NID to parse
;; @conn_list: connection list array
;; @max_conns: max. number of connections to store
;;
;; Parses the connection list of the given widget and stores the list
;; of NIDs.
;;
;; Returns the number of connections, or a negative error code.
;;
proc  snd_hda_get_connections stdcall, nid:dword, conn_list:dword, max_conns:dword   ;Asper: Complete translation!
    locals
       parm          dd ?
       conn_len      dd ?
       conns         dd 0
       shift         db 8
       num_elements  dd 4
       mask          dd 0x7F
       wcaps         dd ?
       prev_nid      dw 1 ;Asper: Hmm.. Probably ALSA bug that it isn't initialized. I suppose to init it with 1.
    endl

    push     ebx ecx edx edi esi
    mov      edi, [conn_list]
    test     edi, edi
    jz       .err_out
    mov      ecx, [max_conns]
    cmp      ecx, 0
    jle      .err_out


    stdcall  get_wcaps, [nid]
    mov      ebx, eax
    mov      [wcaps], eax
    stdcall  get_wcaps_type, ebx
    cmp      eax, AC_WID_VOL_KNB
    je       .conn_list_ok
    test     ebx, AC_WCAP_CONN_LIST
    jnz      .conn_list_ok
  if DEBUG
    mov      esi, emsgConnListNotAvailable
    call     SysMsgBoardStr
    mov      eax, [nid]
    stdcall fdword2str, 3
    call     SysMsgBoardStr
  end if
    xor      eax, eax
    dec      eax
    jmp      .out
  .conn_list_ok:

    snd_hda_param_read  [nid], AC_PAR_CONNLIST_LEN
    mov      [parm], eax

    test     eax, AC_CLIST_LONG
    jz       @f
    ; long form
    mov      [shift], 16
    mov      [num_elements], 2
    mov      [mask], 0x7FFF ;Asper+
  @@:
    and      eax, AC_CLIST_LENGTH
    test     eax, eax
    jz       .out ; no connection

    mov      [conn_len], eax
    cmp      eax, 1
    jne      .multi_conns
    ; single connection
    stdcall  snd_hda_codec_read, [nid], 0, AC_VERB_GET_CONNECT_LIST, 0
    mov      [parm], eax
    cmp      [parm], -1
    jne      @f
    cmp      [ctrl.rirb_error], 0
    jne      @f
    xor      eax, eax
    dec      eax
    jmp      .out
  @@:

    mov      eax, [parm]
    and      eax, [mask]
    stosd
    xor      eax, eax
    inc      eax
    jmp      .out
  .multi_conns:

    ; multi connection
    xor      ecx, ecx
    mov      edx, [num_elements]
  .next_conn:
    mov      eax, ecx
  .mod:
    cmp      eax, edx
    jl       .mod_counted
    sub      eax, edx
    jmp      .mod
  .mod_counted:

    test     eax, eax
    jnz      .l1
    stdcall  snd_hda_codec_read, [nid], 0, AC_VERB_GET_CONNECT_LIST, ecx
    mov      [parm], eax

    cmp      eax, -1
    jne      .l1
    cmp      [ctrl.rirb_error], 0
    jne      .err_out
  .l1:

    mov      eax, 1
    push     ecx
    mov      cl, [shift]
    dec      cl
    shl      eax, cl
    and      eax, [parm]
    pop      ecx
    mov      ebx, eax  ;ranges

    mov      eax, [parm]
    and      eax, [mask] ;val

    test     eax, eax
    jnz      @f
 if DEBUG
    push     eax esi
    mov      esi, emsgInvConnList
    call     SysMsgBoardStr
    mov      eax, [nid]
    stdcall  fdword2str, 1
    call     SysMsgBoardStr

    mov      esi, strSemicolon
    call     SysMsgBoardStr
    mov      eax, ecx
    stdcall  fdword2str, 0
    call     SysMsgBoardStr

    mov      esi, strSemicolon
    call     SysMsgBoardStr
    mov      eax, [parm]
    stdcall  fdword2str, 2
    call     SysMsgBoardStr
    pop      esi eax
 end if
    xor      eax, eax
    jmp      .out
  @@:
    push     ecx
    mov      cl, [shift]
    shr      [parm], cl
    pop      ecx

    test     ebx, ebx
    jz       .range_zero
    ; ranges between the previous and this one
    movzx    esi, word [prev_nid]
    test     esi, esi
    jz       .l2
    cmp      esi, eax
    jl       @f
  .l2:
 if DEBUG
    push     eax esi
    push     esi
    mov      esi, emsgInvDepRangeVal
    call     SysMsgBoardStr
    pop      esi
    push     eax
    mov      eax, esi
    stdcall  fdword2str, 0
    call     SysMsgBoardStr

    mov      esi, strSemicolon
    call     SysMsgBoardStr
    pop      eax
    stdcall  fdword2str, 2
    call     SysMsgBoardStr
    pop      esi eax
 end if
    jmp      .continue
  @@:
    push     ecx
    mov      ecx, esi
    inc      ecx
    mov      ebx, [conns]
  .next_conn2:
    cmp      ebx, [max_conns]
    jl       @f
 if DEBUG
    push     esi
    mov      esi, emsgTooManyConns
    call     SysMsgBoardStr
    pop      esi
 end if
    pop      ecx
    jmp      .err_out
  @@:
    shl      ebx, 1
    push     edi
    add      edi, ebx
    mov      word [edi], cx
    pop      edi
    shr      ebx, 1
    inc      ebx
    inc      ecx
    cmp      ecx, eax
    jle      .next_conn2

    mov      [conns], ebx
    pop      ecx
    jmp      .end_range_test
  .range_zero:

    mov      ebx, [conns]
    cmp      ebx, [max_conns]
    jl       @f
 if DEBUG
    push     esi
    mov      esi, emsgTooManyConns
    call     SysMsgBoardStr
    pop      esi
 end if
    jmp      .err_out
  @@:
    shl      ebx, 1
    push     edi
    add      edi, ebx
    mov      word [edi], ax
    pop      edi
    shr      ebx, 1
    inc      ebx
    mov      [conns], ebx
  .end_range_test:
    mov      [prev_nid], ax
  .continue:
    inc      ecx
    cmp      ecx, [conn_len]
    jl       .next_conn

     mov     eax, [conns]
  .out:
     pop     esi edi edx ecx ebx
     ret
  .err_out:
     pop     esi edi edx ecx ebx
     mov     eax, -1
     ret
endp


; Asper: Have to be realized later, when we will work with such events, but not NOW!
;proc  snd_hda_queue_unsol_events stdcall, res:dword, res_ex:dword
;    push     ebx  edi  esi
;    ...
;    pop      esi  edi  ebx
;    ret
;endp

; This functions also will be later realized.
;proc  process_unsol_events stdcall, work:dword
;proc  init_usol_queue stdcall, bus:dword

;;
;; snd_hda_bus_new - create a HDA bus
;; @card: the card entry
;; @temp: the template for hda_bus information
;; @busp: the pointer to store the created bus instance
;;
;; Returns 0 if successful, or a negative error code.
;;
;proc  snd_hda_bus_new
    ; if we want to support unsolicited events, we have to solve this
    ;    bus->workq = create_singlethread_workqueue(bus->workq_name);
    ; (...)
;    xor   eax, eax
;    ret
;endp

;;
;; snd_hda_codec_init - initialize a HDA codec
;;
;; Returns 0 if successful, or a negative error code.
;;
proc  snd_hda_codec_init    ; We use just one codec (the first found)
     snd_hda_param_read  AC_NODE_ROOT, AC_PAR_VENDOR_ID
     cmp     eax, -1
     jne     @f
     snd_hda_param_read  AC_NODE_ROOT, AC_PAR_VENDOR_ID
  @@:
     mov     [codec.chip_id], ax
     shr     eax, 16
     mov     [codec.vendor_id], ax

     snd_hda_param_read  AC_NODE_ROOT, AC_PAR_SUBSYSTEM_ID
     mov     [codec.subsystem_id], eax

     snd_hda_param_read  AC_NODE_ROOT, AC_PAR_REV_ID
     mov     [codec.revision_id], eax

     stdcall setup_fg_nodes

     mov     eax, [codec.afg]
     test    eax, eax
     jnz     @f

     ;Asper+: try to use another codec if possible [
  if DEBUG
     push    esi
     mov     esi, msgNoAFGFound
     call    SysMsgBoardStr
     pop     esi
  end  if
     push    ecx
     inc     eax
     mov     ecx, [codec.addr]
     shl     eax, cl
     pop     ecx
     cmp     eax, [ctrl.codec_mask]
     jl      .skip_codec
     ;Asper+]

     mov     eax, [codec.mfg]
     test    eax, eax
     jnz     @f
  if DEBUG
     push    esi
     mov     esi, emsgNoAFGorMFGFound
     call    SysMsgBoardStr
     pop     esi
  end  if
  .skip_codec:
     mov     eax, -1
     ret
  @@:

     mov     ebx, eax
     push    ebx
     stdcall read_widget_caps, eax

     cmp     eax, 0
     jge     @f
  if DEBUG
     push    esi
     mov     esi, emsgNoMem
     call    SysMsgBoardStr
     pop     esi
  end  if
     pop     ebx
     mov     eax, -1
     ret
  @@:

     call read_pin_defaults

     cmp     eax, 0
     jge     @f
     pop     ebx
     mov     eax, -1
     ret
  @@:
     mov     eax, [codec.subsystem_id]
     test    eax, eax
     jnz     @f
     stdcall snd_hda_codec_read, ebx, 0, AC_VERB_GET_SUBSYSTEM_ID, 0

  @@:

     ; power up all before initialization
     stdcall snd_hda_set_power_state, ebx, AC_PWRST_D0

     xor     eax, eax
     pop     ebx
     ret
endp


;;
;; snd_hda_codec_configure - (Re-)configure the HD-audio codec
;;
;; Start parsing of the given codec tree and (re-)initialize the whole
;; patch instance.
;;
;; Returns 0 if successful or a negative error code.
;;
proc  snd_hda_codec_configure
     call    get_codec_name
  @@:
     ; call the default parser
     stdcall snd_hda_parse_generic_codec  ;entry point to generic tree parser!!!

     test    eax, eax
     jz      @f
  if DEBUG
     push    esi
     mov     esi, emsgNoParserAvailable
     call    SysMsgBoardStr
     pop     esi
  end if
  @@:
  .out:
     ret
endp


; get_codec_name - store the codec name
proc  get_codec_name
     push    eax  ebx  edi  esi
     mov     eax, [codec.ac_vendor_ids]
     test    eax, eax
     jnz     .get_chip_name
     mov     ax, [codec.vendor_id]
     mov     edi, hda_vendor_ids

  @@:
     mov     ebx, [edi]
     test    ebx, ebx
     jz      .unknown

     cmp     ax, bx
     jne     .next
     mov     eax, [edi+4]
     mov     [codec.ac_vendor_ids], eax
     mov     esi, eax
     call    SysMsgBoardStr
  .get_chip_name:
     stdcall detect_chip, [edi+8]
     pop     esi  edi  ebx  eax
     ret
  .next:
     add     edi, 12
     jmp     @b
  .unknown:
     mov     [codec.ac_vendor_ids], ac_unknown
     mov     [codec.chip_ids], chip_unknown

     mov     esi, chip_unknown
     call    SysMsgBoardStr
     movzx   eax, [codec.chip_id]
     stdcall fdword2str, 2
     call    SysMsgBoardStr
     pop     esi  edi  ebx  eax
     ret
endp


align 4
proc detect_chip stdcall, chip_tab:dword

     push    eax  ebx  edi  esi
     mov     ax, [codec.chip_id]

     mov     edi, [chip_tab]
@@:
     mov     ebx, [edi]
     cmp     ebx, 0xFF
     je      .unknown

     cmp     ax, bx
     jne     .next
     mov     eax, [edi+4]
     mov     [codec.chip_ids], eax
     mov     esi, eax
     call    SysMsgBoardStr
     pop     esi  edi  ebx  eax
     ret
.next:
     add     edi, 8
     jmp     @b
.unknown:
     mov     [codec.chip_ids], chip_unknown
     mov     esi, chip_unknown
     call    SysMsgBoardStr
     movzx   eax, [codec.chip_id]
     stdcall fdword2str, 2
     call    SysMsgBoardStr
     pop     esi  edi  ebx  eax
     ret
endp


;; look for an AFG and MFG nodes
proc setup_fg_nodes
     push    eax  ebx  ecx
     stdcall snd_hda_get_sub_nodes, AC_NODE_ROOT
     mov     ecx, eax
     and     ecx, 0x7FFF ; total_nodes
     mov     ebx, eax
     shr     ebx, 16
     and     ebx, 0x7FFF ; nid

if DEBUG    ;YAHOO
     push    eax esi
     mov     esi, msgSETUP_FG_NODES
     call    SysMsgBoardStr
     mov     eax, ebx
     stdcall fdword2str, 1
     call    SysMsgBoardStr

     mov     esi, strSemicolon
     call    SysMsgBoardStr
     mov     eax, ecx
     stdcall fdword2str, 3
     call    SysMsgBoardStr
     pop     esi eax
end if

  .next:
     test    ecx, ecx
     jz      .l1
     snd_hda_param_read  ebx, AC_PAR_FUNCTION_TYPE
     and     eax, 0xFF

if DEBUG    ;YAHOO
     push    eax esi
     mov     esi, msgFG_TYPE
     call    SysMsgBoardStr
     stdcall fdword2str, 3
     call    SysMsgBoardStr
     pop     esi eax
end if

     cmp     eax, AC_GRP_AUDIO_FUNCTION
     jne     @f

     mov     [codec.afg], ebx
     mov     [codec.function_id], eax
     jmp     .continue
  @@:
     cmp     eax, AC_GRP_MODEM_FUNCTION
     jne     @f

     mov     [codec.mfg], ebx
     mov     [codec.function_id], eax
     jmp     .continue
  @@:
  .continue:
     inc     ebx
     dec     ecx
     jnz     .next
  .l1:
     pop     ecx  ebx  eax
     ret
endp


;======================================================================================
; read widget caps for each widget and store in cache
proc  read_widget_caps stdcall, fg_node:dword
     push    ebx ecx edx edi

     stdcall snd_hda_get_sub_nodes, [fg_node]
     mov     ecx, eax
     and     ecx, 0x7FFF ; total_nodes
     mov     [codec.num_nodes], cx
     mov     ebx, eax
     shr     ebx, 16
     and     ebx, 0x7FFF ; nid
     mov     [codec.start_nid], bx

if DEBUG    ;YAHOO
     push    eax esi
     mov     esi, msgSETUP_FG_NODES
     call    SysMsgBoardStr
     mov     eax, ebx
     stdcall fdword2str, 1
     call    SysMsgBoardStr

     mov     esi, strSemicolon
     call    SysMsgBoardStr
     mov     eax, ecx
     stdcall fdword2str, 3
     call    SysMsgBoardStr
     pop     esi eax
end if

if FDEBUG    ;YAHOO
     push    esi
     mov     esi, msgWCaps
     call    SysMsgBoardStr
     pop     esi
end if

     mov     eax, ecx
     shl     eax, 2
     push    ebx ecx
     call    Kmalloc
     pop     ecx ebx
     test    eax, eax
     jz      .err_out
     mov     [codec.wcaps], eax

     mov     edi, eax
  .next_node:

     snd_hda_param_read  ebx, AC_PAR_AUDIO_WIDGET_CAP
     stosd
     inc     ebx
     dec     ecx
     jnz     .next_node
     pop     edi edx ecx ebx
     xor     eax, eax
     ret
  .err_out:
     pop     edi edx ecx ebx
     xor     eax, eax
     dec     eax
     ret
endp


; read all pin default configurations and save codec->init_pins
proc  read_pin_defaults
     push    ebx ecx edx edi

     movzx   ebx, [codec.start_nid]
     movzx   ecx, [codec.num_nodes]

     ;Asper [
     mov     eax, HDA_PINCFG.sizeof
     mul     cl
     push    ebx ecx
     call    Kmalloc
     pop     ecx ebx
     test    eax, eax
     jz      .err_out
     mov     [codec.init_pins], eax
     mov     [codec.num_pins], 0
     mov     edi, eax
     ;Asper ]

if FDEBUG
   push   eax esi
   mov    esi, msgPinCfgs
   call   SysMsgBoardStr
   pop    esi eax
end if


  .next_node:
     stdcall get_wcaps, ebx
     and  eax, AC_WCAP_TYPE
     shr  eax, AC_WCAP_TYPE_SHIFT

     cmp     eax, AC_WID_PIN
     jne     .continue

     mov     [edi + HDA_PINCFG.nid], bx
     stdcall snd_hda_codec_read, ebx, 0, AC_VERB_GET_CONFIG_DEFAULT, 0
     mov     [edi + HDA_PINCFG.cfg], eax
     add     edi, HDA_PINCFG.sizeof
     inc     [codec.num_pins]

  .continue:
     inc     ebx
     dec     ecx
     jnz     .next_node

     pop     edi edx ecx ebx
     xor     eax, eax
     ret
  .err_out:
     pop     edi edx ecx ebx
     xor     eax, eax
     dec     eax
     ret
endp



; look up the given pin config list and return the item matching with NID
proc  look_up_pincfg stdcall, array:dword, nid:dword
     push    ebx ecx edx
     mov     ecx, [codec.num_pins]
     mov     eax, [array]
     mov     ebx, [nid]
  .next_pin:
     mov     dx,  [eax + HDA_PINCFG.nid]
     cmp     dx,  bx
     je      .out
  .continue:
     add     eax, HDA_PINCFG.sizeof
     dec     ecx
     jnz     .next_pin

     xor     eax, eax
  .out:
     pop     edx ecx ebx
     ret
endp

; write a config value for the given NID
proc  set_pincfg stdcall, nid:dword, cfg:dword
     push    eax  ebx  ecx  edx
     mov     eax, [cfg]
     xor     ebx, ebx
     mov     edx, AC_VERB_SET_CONFIG_DEFAULT_BYTES_0
     mov     ecx, 4
  @@:
     mov     bl,  al
     stdcall snd_hda_codec_write, [nid], 0, edx, ebx
     shr     eax, 8
     inc     edx
     dec     ecx
     jnz     @b
  .l1:
     pop     edx  ecx  ebx  eax
     ret
endp


;;
;; snd_hda_codec_get_pincfg - Obtain a pin-default configuration
;; @codec: the HDA codec
;; @nid: NID to get the pin config
;;
;; Get the current pin config value of the given pin NID.
;; If the pincfg value is cached or overridden via sysfs or driver,
;; returns the cached value.
;;
proc  snd_hda_codec_get_pincfg stdcall, nid:dword
     push    edi
     stdcall look_up_pincfg, [codec.init_pins], [nid]
     test    eax, eax
     jz      @f
     mov     edi, eax
     mov     eax, [edi + HDA_PINCFG.cfg]
  @@:
     pop     edi
     ret
endp

;======================================================================================

;;
;; snd_hda_codec_setup_stream - set up the codec for streaming
;; @nid: the NID to set up
;; @stream_tag: stream tag to pass, it's between 0x1 and 0xf.
;; @channel_id: channel id to pass, zero based.
;; @format: stream format.
;;
proc hda_codec_setup_stream stdcall, nid:dword, stream_tag:dword, channel_id:dword, format:dword
     push    eax
     mov     eax, [nid]
     test    eax, eax
     jnz     @f
     pop     eax
     ret
  @@:
  if DEBUG
     push    esi
     mov     esi, msgHDACodecSetupStream
     call    SysMsgBoardStr
     stdcall fdword2str, 3
     call    SysMsgBoardStr

     mov     esi, msgStream
     call    SysMsgBoardStr
     mov     eax, [stream_tag]
     stdcall fdword2str, 3
     call    SysMsgBoardStr

     mov     esi, msgChannel
     call    SysMsgBoardStr
     mov     eax, [channel_id]
     stdcall fdword2str, 3
     call    SysMsgBoardStr

     mov     esi, msgFormat
     call    SysMsgBoardStr
     mov     eax, [format]
     stdcall fdword2str, 3
     call    SysMsgBoardStr
     pop     esi
  end if
     mov     eax, [stream_tag]
     shl     eax, 4
     or      eax, [channel_id]
     stdcall snd_hda_codec_write, [nid], 0, AC_VERB_SET_CHANNEL_STREAMID, eax

     mov     eax, 1000  ; wait 1 ms
     call    StallExec

     stdcall snd_hda_codec_write, [nid], 0, AC_VERB_SET_STREAM_FORMAT, [format]
     pop     eax
     ret
endp


proc snd_hda_codec_cleanup_stream stdcall, nid:dword
     push    eax
     mov     eax, [nid]
     test    eax, eax
     jz      @f
     pop     eax
     ret
  @@:
  if DEBUG
     push    esi
     mov     esi, msgHDACodecCleanupStream
     call    SysMsgBoardStr
     stdcall fdword2str, 3
     call    SysMsgBoardStr
     pop     esi
  end if
     stdcall snd_hda_codec_write, [nid], 0, AC_VERB_SET_CHANNEL_STREAMID, 0
  if 0  ; keep the format
     mov     eax, 1000000  ; wait 100 ms
     call    StallExec
     stdcall snd_hda_codec_write, [nid], 0, AC_VERB_SET_STREAM_FORMAT, 0
  end if
     pop     eax
     ret
endp


proc  read_pin_cap, nid:dword
     snd_hda_param_read  [nid], AC_PAR_PIN_CAP
     ret
endp


;; read the current volume
proc get_volume_mute stdcall, nid:dword, ch:dword, direction:dword, index:dword
     push    ebx
     mov     ebx, AC_AMP_GET_LEFT
     mov     eax, [ch]
     test    eax, eax
     jz      @f
     mov     ebx, AC_AMP_GET_RIGHT
  @@:
     mov     eax, [direction]
     cmp     eax, HDA_OUTPUT
     jne     @f
     or      ebx, AC_AMP_GET_OUTPUT
     jmp     .l1
  @@:
     or      ebx, AC_AMP_GET_INPUT
  .l1:
     or      ebx, [index]
     stdcall snd_hda_codec_read, [nid], 0, AC_VERB_GET_AMP_GAIN_MUTE, ebx
     and     eax, 0xFF
     pop     ebx
     ret
endp


;; write the current volume in info to the h/w
proc put_volume_mute stdcall, nid:dword, ch:dword, direction:dword, index:dword, val:dword
     push    eax  ebx
     mov     ebx, AC_AMP_SET_LEFT
     mov     eax, [ch]
     test    eax, eax
     jz      @f
     mov     ebx, AC_AMP_SET_RIGHT
  @@:
     mov     eax, [direction]
     cmp     eax, HDA_OUTPUT
     jne     @f
     or      ebx, AC_AMP_SET_OUTPUT
     jmp     .l1
  @@:
     or      ebx, AC_AMP_SET_INPUT
  .l1:
     mov     eax, [index]
     shl     eax, AC_AMP_SET_INDEX_SHIFT
     or      ebx, eax
     or      ebx, [val]
     stdcall snd_hda_codec_write, [nid], 0, AC_VERB_SET_AMP_GAIN_MUTE, ebx
     pop     ebx  eax
     ret
endp


;;
;; snd_hda_codec_amp_update - update the AMP value
;; @nid: NID to read the AMP value
;; @ch: channel (left=0 or right=1)
;; @direction: #HDA_INPUT or #HDA_OUTPUT
;; @idx: the index value (only for input direction)
;; @mask: bit mask to set
;; @val: the bits value to set
;;
;; Update the AMP value with a bit mask.
;; Returns 0 if the value is unchanged, 1 if changed.
;;
;-proc snd_hda_codec_amp_update stdcall, nid:dword, ch:dword, direction:dword, idx:dword, mask:dword, val:dword
;-     push    ebx  edx
;-     mov     eax, [mask]
;-     mov     ebx, [val]
;-     and     ebx, eax
;-     xor     eax, -1
;-     mov     edx, eax
;-     stdcall get_volume_mute, [nid], [ch], [direction], [idx]
;-     and     eax, edx
;-     or      ebx, eax
;-
;-     stdcall put_volume_mute, [nid], [ch], [direction], [idx], ebx
;-     xor     eax, eax
;-     inc     eax
;-     pop     edx  ebx
;-     ret
;-endp


;;
;; snd_hda_codec_amp_stereo - update the AMP stereo values
;; @nid: NID to read the AMP value
;; @direction: #HDA_INPUT or #HDA_OUTPUT
;; @idx: the index value (only for input direction)
;; @mask: bit mask to set
;; @val: the bits value to set
;;
;; Update the AMP values like snd_hda_codec_amp_update(), but for a
;; stereo widget with the same mask and value.
;;
proc snd_hda_codec_amp_stereo stdcall, nid:dword, direction:dword, idx:dword, mask:dword, val:dword
     push    ebx edx
     mov     ebx, [val]
     mov     edx, [mask]
     and     ebx, edx
     stdcall put_volume_mute, [nid], 0, [direction], [idx], ebx
     stdcall put_volume_mute, [nid], 1, [direction], [idx], ebx
     pop     edx  ebx
     ret
endp


;; set power state of the codec
proc snd_hda_set_power_state stdcall, fg:dword, power_state:dword
     push    eax ebx ecx edx
     ; this delay seems necessary to avoid click noise at power down
     mov     ebx, [power_state]
     cmp     ebx, AC_PWRST_D3
     jne     @f
     mov     eax, 100000
     call    StallExec
  @@:
     stdcall snd_hda_codec_read, [fg], 0, AC_VERB_SET_POWER_STATE, ebx
     ;partial workaround for "azx_get_response timeout"
     cmp     ebx, AC_PWRST_D0
     jne     @f

     mov     dx, [codec.vendor_id]
     cmp     dx, 0x14F1

     jne     @f
     mov     eax, 10000
     call    StallExec
  @@:
     movzx   ecx, [codec.num_nodes]
     movzx   edx, [codec.start_nid]
  .next_nid:
     stdcall get_wcaps, edx
     test    eax, AC_WCAP_POWER
     jz      .skip_nid

     stdcall get_wcaps_type, eax
     cmp     ebx, AC_PWRST_D3
     jne     .l1
     cmp     eax, AC_WID_PIN
     jne     .l1
     ;don't power down the widget if it controls
     ;eapd and EAPD_BTLENABLE is set.
     stdcall  read_pin_cap, edx
     test    eax, AC_PINCAP_EAPD
     jz      .l2

     stdcall snd_hda_codec_read, edx, 0, AC_VERB_GET_EAPD_BTLENABLE, 0
     and     eax, 0x02
     test    eax, eax
     jnz     .skip_nid
  .l2:
  .l1:
     stdcall snd_hda_codec_write, edx, 0, AC_VERB_SET_POWER_STATE, ebx
  .skip_nid:
     inc     edx
     dec     ecx
     jnz     .next_nid

     cmp     ebx, AC_PWRST_D0
     jne     .out
     ;wait until codec reaches to D0
     mov     ecx, 500
  .wait_D0:
     stdcall snd_hda_codec_read, [fg], 0, AC_VERB_GET_POWER_STATE, 0
     cmp     eax, ebx
     je      .out
     mov     eax, 1000  ; msleep(1);
     call    StallExec
     dec     ecx
     jnz     .wait_D0
  .out:
     pop     edx ecx ebx eax
     ret
endp


;data

; codec vendors
align 16
msg_Cirrus       db 'Cirrus Logic ',0
msg_Motorola       db 'Motorola ',0
msg_SiliconImage     db 'Silicon Image ',0
msg_Realtek       db 'Realtek ',0
msg_Creative       db 'Creative ',0
msg_IDT        db 'IDT ',0
msg_LSI        db 'LSI ',0
msg_AnalogDevices    db 'Analog Devices ',0
msg_CMedia       db 'C-Media ',0
msg_Conexant       db 'Conexant ',0
msg_Chrontel       db 'Chrontel ',0
msg_LG         db 'LG ',0
msg_Wolfson       db 'Wolfson Microelectronics ',0
msg_Qumranet       db 'Qumranet ',0
msg_SigmaTel       db 'SigmaTel ',0
ac_unknown     db 'unknown manufacturer ',0

chip_unknown   db 'unknown codec id ', 0


; codec vendor labels
align 4
hda_vendor_ids:
     dd    0x1002, msg_ATI, chips_ATI
     dd    0x1013, msg_Cirrus, chips_Cirrus
     dd    0x1057, msg_Motorola, chips_Motorola
     dd    0x1095, msg_SiliconImage, chips_SiliconImage
     dd    0x10de, msg_NVidia, chips_NVidia
     dd    0x10ec, msg_Realtek, chips_Realtek
     dd    0x1102, msg_Creative, chips_Creative
     dd    0x1106, msg_VIA, chips_VIA
     dd    0x111d, msg_IDT, chips_IDT
     dd    0x11c1, msg_LSI, chips_LSI
     dd    0x11d4, msg_AnalogDevices, chips_Analog
     dd    0x13f6, msg_CMedia, chips_CMedia
     dd    0x14f1, msg_Conexant, chips_Conexant
     dd    0x17e8, msg_Chrontel, chips_Chrontel
     dd    0x1854, msg_LG, chips_LG
     dd    0x1aec, msg_Wolfson, chips_Wolfson
     dd    0x1af4, msg_Qumranet, chips_Qumranet   ; Qemu 0.14
     dd    0x434d, msg_CMedia, chips_CMedia
     dd    0x8086, msg_Intel, chips_Intel
     dd    0x8384, msg_SigmaTel, chips_SigmaTel
     dd    0 ; terminator

align 16             ;known codecs
chips_ATI            dd 0xAA01, chip_ATIR6XX
                     dd 0xFF

chips_Cirrus         dd 0xFF
chips_Motorola       dd 0xFF

chips_SiliconImage   dd 0x1392, chip_SI1392
                     dd 0xFF

chips_NVidia         dd 0x0002, chip_MCP78
                     dd 0xFF

chips_Realtek        dd 0x0262, chip_ALC262
                     dd 0x0268, chip_ALC268
                     dd 0x0269, chip_ALC269
                     dd 0x0272, chip_ALC272
                     dd 0x0662, chip_ALC662
                     dd 0x0663, chip_ALC663
                     dd 0x0883, chip_ALC883
                     dd 0x0887, chip_ALC887
                     dd 0x0888, chip_ALC888
                     dd 0x0889, chip_ALC889
                     dd 0xFF

chips_Creative       dd 0xFF

chips_VIA            dd 0xE721, chip_VT1708B_1
                     dd 0x0397, chip_VT17085_0
                     dd 0xFF

chips_IDT            dd 0xFF

chips_LSI            dd 0x1039, chip_LSI1039
                     dd 0x1040, chip_LSI1040
                     dd 0x3026, chip_LSI3026
                     dd 0x3055, chip_LSI3055
                     dd 0xFF

chips_Analog         dd 0x1986, chip_AD1986A
                     dd 0x198B, chip_AD198B
                     dd 0xFF

chips_CMedia         dd 0xFF

chips_Conexant       dd 0x5045, chip_CX20549
                     dd 0x5051, chip_CX20561
                     dd 0xFF

chips_Chrontel       dd 0xFF
chips_LG             dd 0xFF
chips_Wolfson        dd 0xFF
chips_Intel          dd 0xFF

chips_Qumranet       dd 0x0010, chip_HDA_OUTPUT
                     dd 0x0020, chip_HDA_DUPLEX
                     dd 0xFF

chips_SigmaTel       dd 0x7680, chip_STAC9221
                     dd 0x7682, chip_STAC9221_A2
                     dd 0xFF

align 16
;AnalogDevices
chip_AD1986A         db 'AD1986A',13,10,0
chip_AD198B          db 'AD198B',13,10,0

;ATI
chip_ATIR6XX         db 'ATIR6XX',13,10,0

;Silicon Image
chip_SI1392          db 'SI1392',13,10,0

;NVidia
chip_MCP78           db 'MCP78',13,10,0

;Realtek
chip_ALC262          db 'ALC262',13,10,0
chip_ALC268          db 'ALC268',13,10,0
chip_ALC269          db 'ALC269',13,10,0
chip_ALC272          db 'ALC272',13,10,0
chip_ALC662          db 'ALC662',13,10,0
chip_ALC663          db 'ALC663',13,10,0
chip_ALC883          db 'ALC883',13,10,0
chip_ALC887          db 'ALC887',13,10,0
chip_ALC888          db 'ALC888',13,10,0
chip_ALC889          db 'ALC889',13,10,0

;Sigmatel
chip_STAC9221        db 'STAC9221',13,10,0
chip_STAC9221_A2     db 'STAC9221_A2',13,10,0

;VIA
chip_VT1708B_1       db 'VT1708B_1',13,10,0
chip_VT17085_0       db 'VT17085_0',13,10,0

;Conexant
chip_CX20549         db 'CX20549',13,10,0
chip_CX20561         db 'CX20561',13,10,0

;Qumranet
chip_HDA_OUTPUT      db 'HDA-OUTPUT',13,10,0
chip_HDA_DUPLEX      db 'HDA-DUPLEX',13,10,0

;LSI
chip_LSI1039         db '1039 (Agere Systems HDA Modem)',13,10,0
chip_LSI1040         db '1040 (Agere Systems HDA Modem)',13,10,0
chip_LSI3026         db '3026 (Agere Systems HDA Modem)',13,10,0
chip_LSI3055         db '3055 (Agere Systems HDA Modem)',13,10,0
