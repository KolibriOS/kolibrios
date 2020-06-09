

struc mp3dec_frame_info {

        .frame_bytes    dd ?    ; signed int
        .frame_offset   dd ?    ; signed int
        .channels       dd ?    ; signed int
        .hz             dd ?    ; signed int
        .layer          dd ?    ; signed int
        .bitrate_kbps   dd ?    ; signed int

}

; mp3d                  rb 8192

;        invoke  mp3dec_init, mp3d
;        test    eax, eax
;        jz

; mp3dec_frame_info     info
; pcm                   rw MINIMP3_MAX_SAMPLES_PER_FRAME

;        invoke  mp3dec_decode_frame, mp3d, input_buf, [buf_size], pcm, info
;        mov     [samples], eax