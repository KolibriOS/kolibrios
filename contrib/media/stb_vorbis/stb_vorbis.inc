
struc vorb_alloc_buffer {

        .ptr    dd ?
        .length dd ?

}

struc stb_vorbis_info {

        .sample_rate                    dd ?    ; unsigned int
        .channels                       dd ?    ; signed int

        .setup_memory_required          dd ?
        .setup_temp_memory_required     dd ?
        .temp_memory_required           dd ?

        .max_frame_size                 dd ?

}

;        invoke  pushdata_open, input_buf, [buf_size], consumed, error, alloc_buffer
;        test    eax, eax
;        jz
;       mov [vorb], eax

;        invoke  pushdata_decode, [vorb], input_buf, [buf_size], numchannels, float_samples, numsamples
;        mov     [bytes_consumed], eax

;        invoke  vorb_convert_channels_short_interleaved, numchannels, pcm, numchannels, float_samples, int d_offset, int len

;consumed       dd ?
;error          dd ?
;numchannels    dd ?
;numsamples     dd ?
;float_samples  rd ?
;pcm            rw VORB_MAX_SAMPLES_PER_FRAME

