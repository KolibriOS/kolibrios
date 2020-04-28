;====================================================================
;===== éäçé èêÖÑèêéëåéíêÄ ===========================================
;====================================================================
scr_window:
.start:
.red:
        call    .draw_window
.still:
        wait_event .red, .close
        jmp     .still

.draw_window:
        start_draw_window
        xor     eax,eax
        movsx   ebx,word [scr.width]
        movsx   ecx,word [scr.height]
        ;xor     edx,edx
        mov     edx,0x01000000
        xor     esi,esi
        xor     edi,edi
        mcall

        mov     eax,7
        mov     ebx,dword [scr_buf.ptr]
        mov     ecx,dword [scr_buf.size]
        xor     edx,edx
        mcall

        mov     eax,13
        mov     ebx,19*65536+231
        mov     ecx,19*65536+10
        mov     edx,0x00FF00
        mcall

        mov     edi,.label
        call    draw_label

        stop_draw_window
ret
.close:
        mcall   60,2,[app.pid],messages.act_window,2
        jmp	close

.label label 20,20,0x80000000,keyforexit
