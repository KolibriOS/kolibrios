;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Universal Interface for Intel High Definition Audio Codec  ;
;                                                            ;
; Generic widget tree parser                                 ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; widget node for parsing
struc HDA_GNODE
{
	.nid		    dw ?    ;NID of this widget
	.nconns 	    dw ?    ;number of input connections
	.conn_list	    dd ?
	.slist		    dw ?    ;temporary list
			    dw ?

	.wid_caps	    dd ?    ;widget capabilities
	.type		    db ?    ;widget type
	.pin_ctl	    db ?    ;pin controls
	.checked	    db ?    ;the flag indicates that the node is already parsed
	.pin_caps	    dd ?    ;pin widget capabilities
	.def_cfg	    dd ?    ;default configuration
	.amp_out_caps	    dd ?    ;AMP out capabilities
	.amp_in_caps	    dd ?    ;AMP in capabilities
	.next		    dd ? ;        struct list_head list
	.sizeof:
}

virtual at 0
	HDA_GNODE  HDA_GNODE
end virtual

struc HDA_GSPEC
{
	.dac_node	    dd ?    ;DAC node
			    dd ?
	.out_pin_node	    dd ?    ;Output pin (Line-Out) node
			    dd ?

	.def_amp_in_caps    dd ?
	.def_amp_out_caps   dd ?

;        .pcm_rec            dd ?    ;PCM information
	.nid_list	    dd 0    ;list of widgets
}

struc VOLUME_CTL
{
	.out_amp_node	    dd 0    ;Asper+ : To get/set volume
	.num_steps	    db ?    ; num_steps=NumSteps+1
	.step_size	    db ?    ; step_size=StepSize+1
	.maxDb		    dd ?    ; Max volume in Db.   maxDb=(num_steps*step_size/4*100)
}

; retrieve the default device type from the default config value
proc defcfg_type stdcall, node:dword
	push	 edx
	mov	 edx, [node]
	mov	 eax, [edx + HDA_GNODE.def_cfg]
	and	 eax, AC_DEFCFG_DEVICE
	shr	 eax, AC_DEFCFG_DEVICE_SHIFT
	pop	 edx
	ret
endp

proc defcfg_location stdcall, node:dword
	push	 edx
	mov	 edx, [node]
	mov	 eax, [edx + HDA_GNODE.def_cfg]
	and	 eax, AC_DEFCFG_LOCATION
	shr	 eax, AC_DEFCFG_LOCATION_SHIFT
	pop	 edx
	ret
endp

proc defcfg_port_conn stdcall, node:dword
	push	 edx
	mov	 edx, [node]
	mov	 eax, [edx + HDA_GNODE.def_cfg]
	and	 eax, AC_DEFCFG_PORT_CONN
	shr	 eax, AC_DEFCFG_PORT_CONN_SHIFT
	pop	 edx
	ret
endp

proc defcfg_color stdcall, node:dword
	push	 edx
	mov	 edx, [node]
	mov	 eax, [edx + HDA_GNODE.def_cfg]
	and	 eax, AC_DEFCFG_COLOR
	shr	 eax, AC_DEFCFG_COLOR_SHIFT
	pop	 edx
	ret
endp

; destructor
proc snd_hda_generic_free
	push	eax ebx edx edi
	; free all widgets
	mov	ebx, [spec.nid_list]  ; ebx = 1st node address
	test	ebx, ebx
	jz	.out
	mov	edx, [ebx + HDA_GNODE.next]  ;edx = 2nd node address

.next:
	test	edx, edx
	jz	.free_head

	mov	eax, [edx + HDA_GNODE.conn_list]
	lea	edi, [edx + HDA_GNODE.slist]
	cmp	eax, edi
	je	@f
	pusha
	invoke	Kfree  ;free conn_list
	popa
@@:
	mov	eax, edx
	mov	edx, [edx + HDA_GNODE.next]
	pusha
	invoke	Kfree  ;free node
	popa
	jmp	.next
.free_head:
	mov	eax, [spec.nid_list]
	pusha
	invoke	Kfree  ;free the very 1st node in the list
	popa
	mov	[spec.nid_list], 0
.out:
	pop	edi edx ebx eax
	ret
endp


; add a new widget node and read its attributes
proc add_new_node stdcall, nid:dword
	push	ebx ecx edx edi esi

	mov	eax, HDA_GNODE.sizeof
	invoke	Kmalloc
	test	eax, eax
	jz	.err_out   ; Not enough memory

	mov	edx, eax
;Asper+ [
	mov	edi, edx
	xor	eax, eax
	mov	ecx, HDA_GNODE.sizeof
	rep	stosb
;Asper+ ]

	mov	eax, [nid]
	mov	word [edx + HDA_GNODE.nid], ax
	stdcall get_wcaps, eax
	mov	[edx + HDA_GNODE.wid_caps], eax
	mov	ebx, eax
	stdcall get_wcaps_type, eax
	mov	byte [edx + HDA_GNODE.type], al

	mov	eax, HDA_MAX_CONNECTIONS*2  ;HDA_MAX_CONNECTIONS * sizeof(word)
	push	ebx ecx edx
	invoke	Kmalloc    ;malloc temporary conn_list
	pop	edx ecx ebx
	mov	edi, eax

	test	ebx, AC_WCAP_CONN_LIST
	jz	.no_conn_list

	stdcall snd_hda_get_connections, [nid], edi, HDA_MAX_CONNECTIONS
	mov	ecx, eax
	cmp	ecx, 0
	jge	@f

	mov	eax, edx
	pusha
	invoke	Kfree	   ;free node
	popa
	mov	eax, ecx
	jmp	.out
.no_conn_list:

	xor	ecx, ecx
@@:
	cmp	ecx, 2	   ;nconns <= ARRAY_SIZE(node->slist) ?
	jg	@f

	lea	eax, [edx + HDA_GNODE.slist]
	mov	[edx + HDA_GNODE.conn_list], eax
	jmp	.set_conn_list
@@:
	mov	eax, ecx
	shl	ecx, 1
	push	ebx ecx edx edi
	invoke	Kmalloc    ;malloc conn_list
	pop	edi edx ecx ebx
	shr	ecx, 1
	test	eax, eax
	jnz	@f

	mov	eax, edi
	pusha
	invoke	Kfree	   ;free temporary conn_list
	popa
	jmp	.err_out
@@:
	mov	[edx + HDA_GNODE.conn_list], eax
.set_conn_list:
	mov	[edx + HDA_GNODE.nconns], cx
	push	edi
	mov	esi, edi
	mov	edi, eax
	rep	movsw
	pop	edi


	mov	al, byte [edx + HDA_GNODE.type]
	test	al, AC_WID_PIN
	jz	@f
;Asper+ [
	cmp	al, AC_WID_VENDOR
	je	@f
;Asper+ ]


	stdcall read_pin_cap, [nid]
	mov	[edx + HDA_GNODE.pin_caps], eax
	stdcall snd_hda_codec_read, [nid], 0, AC_VERB_GET_PIN_WIDGET_CONTROL, 0
	mov	byte [edx + HDA_GNODE.pin_ctl], al
	stdcall snd_hda_codec_get_pincfg, [nid]
	mov	[edx + HDA_GNODE.def_cfg], eax
@@:

	xor	eax, eax
	test	ebx, AC_WCAP_OUT_AMP
	jz	.no_out_amp
	test	ebx, AC_WCAP_AMP_OVRD
	jz	@f
	snd_hda_param_read [nid], AC_PAR_AMP_OUT_CAP
@@:
	test	eax, eax
	jnz	@f
	mov	eax, [spec.def_amp_out_caps]
@@:
	mov	[edx + HDA_GNODE.amp_out_caps], eax
.no_out_amp:

;;Asper+: Beeper [
;        pusha
;        mov     bl, byte [edx + HDA_GNODE.type]
;        cmp     bl, AC_WID_BEEP
;        jne     .not_beeper
;
;        mov     ebx, [nid]
;        mov     [codec.beeper_nid], bx
;
;        test    eax, eax
;        jz      .no_beeper_amp
;        ;set beep amplifier here
;        stdcall unmute_output, edx
;.no_beeper_amp:
;        ;try to beep here
;        stdcall snd_hda_codec_read, [nid], 0, AC_VERB_GET_BEEP_CONTROL, 0 ;eax
;if DEBUG
;        push    eax esi
;        mov     esi, msgBeeperNid
;        invoke  SysMsgBoardStr
;        push    eax
;        mov     eax, [nid]
;        stdcall fdword2str, 2
;        invoke  SysMsgBoardStr
;
;        mov     esi, msgBeeperValue
;        invoke  SysMsgBoardStr
;        pop     eax
;        stdcall fdword2str, 2
;        invoke  SysMsgBoardStr
;
;        mov     esi, msgBeepNow
;        invoke  SysMsgBoardStr
;        pop     esi eax
;end if
;        mov     ecx, 256*1
;.next_tone:
;        dec     ecx
;        movzx   ebx, [esi + HDA_GNODE.nid]
;        stdcall snd_hda_codec_write, [nid], 0, AC_VERB_SET_BEEP_CONTROL, ecx
;        ;mov     eax, 0x8000
;        ;stdcall StallExec
;        test    ecx, ecx
;        jnz     .next_tone
;.end_beep:
;        stdcall snd_hda_codec_read, [nid], 0, AC_VERB_GET_BEEP_CONTROL, 0 ;eax
;if DEBUG
;        ;push    eax esi
;        mov     esi, msgBeeperValue
;        invoke  SysMsgBoardStr
;        stdcall  fdword2str, 2
;        invoke   SysMsgBoardStr
;        ;pop      esi eax
;end if
;.not_beeper:
;        popa
;;Asper+: Beeper ]

	xor	eax, eax
	test	ebx, AC_WCAP_IN_AMP
	jz	.no_in_amp
	test	ebx, AC_WCAP_AMP_OVRD
	jz	@f
	snd_hda_param_read [nid], AC_PAR_AMP_IN_CAP
@@:
	test	eax, eax
	jnz	@f
	mov	eax, [spec.def_amp_in_caps]
@@:
	mov	[edx + HDA_GNODE.amp_in_caps], eax
.no_in_amp:

	mov	esi, [spec.nid_list]
	test	esi, esi
	jnz	@f
	mov	[spec.nid_list], edx
	jmp	.out
@@:

	;Asper+: Sort pins by DA:Sequence during tree building [
	mov	ecx, esi
	movzx	ebx, byte [edx + HDA_GNODE.def_cfg]
	push	edi
.next_node:
	cmp	[esi + HDA_GNODE.type], AC_WID_PIN
	jne	@f
	cmp	[edx + HDA_GNODE.type], AC_WID_PIN
	je	.pin

	mov	edi, [spec.nid_list]
	cmp	[edi + HDA_GNODE.type], AC_WID_PIN
	jne	.not_pin
	mov	[edx + HDA_GNODE.next], edi
.head:						   ;CleverMouse+
	mov	[spec.nid_list], edx
	pop	edi
	jmp	.out
.pin:
	movzx	edi, byte [esi + HDA_GNODE.def_cfg]
	cmp	edi, ebx
	jle	@f
.not_pin:
	mov	[edx + HDA_GNODE.next], esi
	cmp	esi, [spec.nid_list]			;CleverMouse+
	jz	.head					;CleverMouse+
	mov	esi, ecx
	jmp	.insert
@@:
	mov	eax, [esi + HDA_GNODE.next]
	test	eax, eax
	jz	.insert
	mov	ecx, esi
	mov	esi, eax
	jmp	.next_node
.insert:
	mov	[esi + HDA_GNODE.next], edx
	pop	edi
	;Asper+ ]

.out:
	mov	eax, edi
	pusha
	invoke	Kfree	  ;free temporary conn_list
	popa
	xor	eax, eax
	pop	esi edi edx ecx ebx
	ret

.err_out:
	mov	eax, edx
	pusha
	invoke	Kfree	  ;free node
	popa
	xor	eax, eax
	dec	eax
	pop	esi edi edx ecx ebx
	ret
endp



; build the AFG subtree
proc build_afg_tree
	push	ebx ecx edx

	mov	ebx, [codec.afg]
	snd_hda_param_read  ebx, AC_PAR_AMP_OUT_CAP

	mov	[spec.def_amp_out_caps], eax
	snd_hda_param_read  ebx, AC_PAR_AMP_IN_CAP
	mov	[spec.def_amp_in_caps], eax

	stdcall snd_hda_get_sub_nodes, ebx
	mov	ecx, eax
	and	ecx, 0xFFFF    ;ecx = nodes number
	mov	edx, eax
	shr	edx, 16        ;eax = address of the first nid

	test	edx, edx
	jz	@f
	cmp	ecx, 0
	jge	.nid_ok
@@:
if  FDEBUG
	push	esi
	mov	esi, emsgInvalidAFGSubtree
	invoke	SysMsgBoardStr
	pop	esi
end if
	xor	eax, eax
	dec	eax
	jmp	.out
.nid_ok:

	; parse all nodes belonging to the AFG
.next_node:
	test	ecx, ecx
	jz	.build_done

	stdcall add_new_node, edx
	test	eax, eax
	jnz	.out
	inc	edx
	dec	ecx
	jmp	.next_node
.build_done:
	xor	eax, eax
.out:
	pop	edx ecx ebx
	ret
endp

;Asper+[
proc print_afg_tree_nodes
	push	eax esi edi
	mov	esi, msgNodeSeq
	invoke	SysMsgBoardStr

	mov	edi, [spec.nid_list]
	test	edi, edi
	jz	.out
.next_node:
	movzx	eax, word [edi + HDA_GNODE.nid]
	mov	esi, msgNID
	invoke	SysMsgBoardStr
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr

	mov	eax, [edi + HDA_GNODE.next]
	test	eax, eax
	jz	.out

	mov	edi, eax
	jmp	.next_node
.out:
	pop	edi esi eax
	ret
endp
;Asper+]

; look for the node record for the given NID
proc hda_get_node stdcall, nid:dword
	push	ebx edx esi
	movzx	ebx, word [nid]
	mov	esi, [spec.nid_list]
	test	esi, esi
	jz	.out

.next_node:
	mov	edx, [esi + HDA_GNODE.next]
	test	edx, edx   ;Asper+
	jz	.not_found ;Asper+
	mov	ax, word [esi + HDA_GNODE.nid]
	cmp	ax, bx
	je	.out
	mov	esi, edx
	jmp	.next_node

.not_found: ;Asper+
	xor	esi, esi
.out:
	mov	eax, esi
	pop	esi edx ebx
	ret
endp

;Asper+[
proc set_eapd stdcall, node:dword ;nid:dword, on:dword
	push	eax ebx esi
	mov	esi, [node]
	cmp	[esi + HDA_GNODE.type], AC_WID_PIN
	jne	.out
	; eapd capable?
	test	[esi + HDA_GNODE.pin_caps], AC_PINCAP_EAPD
	jz	.out
	;stdcall snd_hda_codec_read, ebx, 0, AC_VERB_GET_EAPD_BTLENABLE, AC_EAPDBTL_EAPD
	;or      eax, AC_EAPDBTL_EAPD
	movzx	ebx, [esi + HDA_GNODE.nid]
	stdcall snd_hda_codec_write, ebx, 0, AC_VERB_SET_EAPD_BTLENABLE, AC_EAPDBTL_EAPD ;eax
if  DEBUG
	push	eax esi
	mov	esi, msgEnableEAPD
	invoke	SysMsgBoardStr
	mov	eax, ebx
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr
	pop	esi eax
      end if
.out:
	pop	esi ebx eax
	ret
endp
;Asper+]

; unmute (and set max vol) the output amplifier
proc unmute_output stdcall, node:dword
	push	ebx ecx edx esi
	mov	esi, [node]
	test	[esi + HDA_GNODE.wid_caps], AC_WCAP_OUT_AMP
	jz	.out
	movzx	eax, word [esi + HDA_GNODE.nid]
if  DEBUG
	push	esi
	mov	esi, msgUnmuteOut
	invoke	SysMsgBoardStr
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr
	pop	esi
end if

	stdcall set_eapd, esi ;Asper+: set EAPD if exist

	mov	ebx, eax
	mov	eax, [esi + HDA_GNODE.amp_out_caps]
	mov	ecx, eax

	and	eax, AC_AMPCAP_NUM_STEPS
	shr	eax, AC_AMPCAP_NUM_STEPS_SHIFT

	stdcall snd_hda_codec_amp_stereo, ebx, HDA_OUTPUT, 0, 0xFF, eax

	and	ecx, AC_AMPCAP_STEP_SIZE
	shr	ecx, AC_AMPCAP_STEP_SIZE_SHIFT

	test	al, al
	jz	.out
if  DEBUG
	push	eax esi
	mov	esi, msgAmpVal
	invoke	SysMsgBoardStr
	stdcall fdword2str, 1
	invoke	SysMsgBoardStr

	mov	esi, strSemicolon
	invoke	SysMsgBoardStr
	mov	eax, ecx
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr
	pop	esi eax
end if
	mov	[volume.out_amp_node], esi
	mov	[volume.num_steps], al
	mov	[volume.step_size], cl
	mul	cl
	imul	eax, (100/4)
	mov	[volume.maxDb], eax
.out:
	xor	eax, eax
	pop	esi edx ecx ebx
	ret
endp

; unmute (and set max vol) the input amplifier
proc unmute_input stdcall, node:dword, index:dword
	push	ecx edx esi
	test	[esi + HDA_GNODE.wid_caps], AC_WCAP_IN_AMP
	jz	.out
	and	[index], 0xF ;Asper+ : Ranger
	mov	esi, [node]
	movzx	eax, word [esi + HDA_GNODE.nid]
if  DEBUG
	push	eax esi
	mov	esi, msgUnmuteIn
	invoke	SysMsgBoardStr
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr
	mov	esi, msgIdx
	invoke	SysMsgBoardStr
	mov	eax, [index]
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr
	pop	esi eax
end if

	mov	edx, [esi + HDA_GNODE.amp_in_caps]
	mov	ecx, edx

	and	edx, AC_AMPCAP_NUM_STEPS
	shr	edx, AC_AMPCAP_NUM_STEPS_SHIFT

	stdcall snd_hda_codec_amp_stereo, eax, HDA_INPUT, [index], 0xFF, edx
.out:
	xor	eax, eax
	pop	esi edx ecx
	ret
endp


; select the input connection of the given node.
proc select_input_connection stdcall, node:dword, index:dword
	push	ebx esi
	mov	esi, [node]
	movzx	eax, word [esi + HDA_GNODE.nid]
	mov	ebx, [index]
if DEBUG
	mov	esi, msgConnect
	invoke	SysMsgBoardStr
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr

	mov	esi, msgIdx
	invoke	SysMsgBoardStr
	push	eax
	mov	eax, ebx
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr
	pop	eax
end if
	stdcall snd_hda_codec_write, eax, 0, AC_VERB_SET_CONNECT_SEL, ebx
	pop	esi ebx
	ret
endp


; clear checked flag of each node in the node list
proc clear_check_flags
	push	eax esi
	mov	esi, [spec.nid_list]
	test	esi, esi
	jz	.out
.next_node:
	mov	byte [esi + HDA_GNODE.checked], 0
	mov	eax, [esi + HDA_GNODE.next]
	test	eax, eax
	jz	.out
	mov	esi, eax
	jmp	.next_node
.out:
	pop	esi eax
	ret
endp

;
; parse the output path recursively until reach to an audio output widget
;
; returns 0 if not found, 1 if found, or a negative error code.
;
proc parse_output_path stdcall, node:dword, dac_idx:dword
	push	ebx ecx edx esi
	mov	esi, [node]
	mov	al, byte [esi + HDA_GNODE.checked]
	test	al, al
	jnz	.ret_zero

	mov	byte [esi + HDA_GNODE.checked], 1

	mov	al, byte [esi + HDA_GNODE.type]
	cmp	al, AC_WID_AUD_OUT
	jne	.not_wid_aud_out

	movzx	eax, word [esi + HDA_GNODE.nid]
	mov	ebx, [esi + HDA_GNODE.wid_caps]
	test	ebx, AC_WCAP_DIGITAL
	jz	@f
if DEBUG
	push	esi
	mov	esi, msgSkipDigitalOutNode
	invoke	SysMsgBoardStr
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr
	pop	esi
end if
	jmp	.ret_zero
@@:
if DEBUG
	push	eax esi
	mov	esi, msgAudOutFound
	invoke	SysMsgBoardStr
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr
	pop	esi eax
end if

	push	eax
	stdcall unmute_output, esi ;Asper+
	pop	eax
	mov	ecx, [dac_idx]
	shl	ecx, 2
	push	eax
	mov	eax, [spec.dac_node+ecx]
	test	eax, eax
	pop	eax
	jz	@f
	; already DAC node is assigned, just unmute & connect
	cmp	eax, [node]
	je	.ret_one
	jmp	.ret_zero
@@:
	mov	ecx, [dac_idx]
	shl	ecx, 2
	mov	[spec.dac_node+ecx], eax
	jmp	.ret_one   ;found
.not_wid_aud_out:
	movzx	ebx, [esi + HDA_GNODE.nconns]
	xor	ecx, ecx
	mov	edx, [esi + HDA_GNODE.conn_list]
	test	ebx, ebx
	jz	.ret_zero
.next_node:
	stdcall hda_get_node, [edx]
	test	eax, eax
	jz	.continue

	stdcall parse_output_path, eax, [dac_idx]

	cmp	[esi + HDA_GNODE.nconns], 1
	jle	@f
	stdcall select_input_connection, esi, ecx
@@:
;UNSUPPORTED YET!        stdcall unmute_input, esi, ecx
	stdcall unmute_output, esi
	jmp	.ret_one

.continue:
	add	edx, 2
	inc	ecx
	cmp	ecx, ebx
	jl	.next_node
.ret_zero:
	xor	eax, eax
	pop	esi edx ecx ebx
	ret
.ret_one:
	xor	eax, eax
	inc	eax
.ret: ;Asper+
	pop	esi edx ecx ebx
	ret
endp

; Look for the output PIN widget with the given jack type
; and parse the output path to that PIN.
;
; Returns the PIN node when the path to DAC is established.
proc parse_output_jack stdcall, jack_type:dword
	push	edx esi

	mov	esi, [spec.nid_list]
	test	esi, esi
	jz	.ret_zero
.next_pin:
	cmp	[esi + HDA_GNODE.type], AC_WID_PIN
	jne	.continue

	; output capable?
	mov	eax, [esi + HDA_GNODE.pin_caps]
	test	eax, AC_PINCAP_OUT
	jz	.continue

	stdcall defcfg_port_conn, esi
	cmp	eax, AC_JACK_PORT_NONE
	je	.continue  ;unconnected

	mov	edx, [jack_type]
	cmp	edx, 0
	jl	@f

	stdcall defcfg_type, esi
	cmp	edx, eax
	jne	.continue

	test	[esi + HDA_GNODE.wid_caps], AC_WCAP_DIGITAL
	jnz	.continue ; skip SPDIF
@@:
	push	eax
	movzx	eax, [esi + HDA_GNODE.nid]
	stdcall snd_hda_enable_pin_sense, eax, eax	;Asper+: enable unsolicited events for the output pin
	pop	eax

if DEBUG
	pusha
;        push    esi
;        mov     esi, msgPin_Nid
;        invoke  SysMsgBoardStr
;        pop     esi
	movzx	eax, [esi + HDA_GNODE.nid]
	movzx	ebx, [esi + HDA_GNODE.pin_ctl]
	mov	ecx, [esi + HDA_GNODE.pin_caps]
	mov	edx, [esi + HDA_GNODE.def_cfg]
	mov	edi, [esi + HDA_GNODE.amp_out_caps]
	mov	esi, msgPin_Nid
	invoke	SysMsgBoardStr
	stdcall fdword2str, 3
	invoke	SysMsgBoardStr

	mov	esi, msgPin_Ctl
	invoke	SysMsgBoardStr
	mov	eax, ebx
	stdcall fdword2str, 2
	invoke	SysMsgBoardStr

	mov	esi, msgPin_Caps
	invoke	SysMsgBoardStr
	mov	eax, ecx
	stdcall fdword2str, 2
	invoke	SysMsgBoardStr

	mov	esi, msgDef_Cfg
	invoke	SysMsgBoardStr
	mov    eax, edx
	stdcall fdword2str, 2
	invoke	SysMsgBoardStr

	mov    esi, msgAmp_Out_Caps
	invoke	SysMsgBoardStr
	mov    eax, edi
	stdcall fdword2str, 2
	invoke	SysMsgBoardStr
	popa
end if
	; output as default?
;        test    [esi + HDA_GNODE.pin_ctl], AC_PINCTL_OUT_EN
;        jz      .continue
.use_dac0:
	cmp	[spec.dac_node], 0
	jne	.use_dac1

	stdcall clear_check_flags
	stdcall parse_output_path, esi, 0

	test	eax, eax
	jnz	@f
	mov	edx, [spec.out_pin_node]
	test	edx, edx
	jz	@f
.use_dac1:
	stdcall clear_check_flags
	stdcall parse_output_path, esi, 1
@@:
	cmp	eax, 0
	jle	.l1

	; unmute the PIN output
	stdcall unmute_output, esi
	; set PIN-Out enable
	xor	edx, edx
	test	[esi + HDA_GNODE.pin_caps], AC_PINCAP_HP_DRV
	jz	@f
	mov	edx, AC_PINCTL_HP_EN
@@:
	or	edx, AC_PINCTL_OUT_EN
	movzx	eax, [esi + HDA_GNODE.nid]
	stdcall snd_hda_codec_write, eax, 0, AC_VERB_SET_PIN_WIDGET_CONTROL, edx
	mov	eax, esi
	jmp	.out
.l1:
.continue:
	mov	edx, [esi + HDA_GNODE.next]
	test	edx, edx
	jz	.ret_zero
	mov	esi, edx
	jmp	.next_pin
.ret_zero:
	xor	eax, eax
.out:
	pop	esi edx
	ret
endp


; parse outputs
proc parse_output
	push	edx
	; Look for the output PIN widget
	;
	; first, look for the line-out pin
	stdcall parse_output_jack, AC_JACK_LINE_OUT
	test	eax, eax
	jz	@f
	mov	[spec.out_pin_node], eax   ; found, remember the PIN node
	jmp	.l1
@@:
	; if no line-out is found, try speaker out
	stdcall parse_output_jack, AC_JACK_SPEAKER
	test	eax, eax
	jz	.l1
	mov	[spec.out_pin_node], eax   ; found, remember the PIN node
.l1:
	; look for the HP-out pin
	stdcall parse_output_jack, AC_JACK_HP_OUT
	test	eax, eax
	jz	.l2

	mov	edx, [spec.out_pin_node]
	test	edx, edx
	jnz	@f
	mov	[spec.out_pin_node], eax
	jmp	.l2
@@:
	mov	[spec.out_pin_node+4], eax
.l2:
	mov	edx, [spec.out_pin_node]
	test	edx, edx
	jnz	@f
	; no line-out or HP pins found,
	; then choose for the first output pin
	stdcall parse_output_jack, -1

	mov	[spec.out_pin_node], eax
	test	eax, eax
	jnz	@f
if DEBUG
	push	esi
	mov	esi, emsgNoProperOutputPathFound
	invoke	SysMsgBoardStr
	pop	esi
end if
@@:
	pop	edx
	xor	eax, eax
	ret
endp


;(...)  Skip functions for the input (capture is not supported).

; the generic parser
proc snd_hda_parse_generic_codec
	mov	eax, [codec.afg]
	test	eax, eax
	jz	.out

	stdcall build_afg_tree
	cmp	eax, 0
	jl	.error

if  FDEBUG
	stdcall print_afg_tree_nodes ;Asper+
end if

	stdcall parse_output
	xor	eax, eax
.out:
	ret
.error:
	stdcall snd_hda_generic_free
	ret
endp


; some data
spec HDA_GSPEC
volume VOLUME_CTL
