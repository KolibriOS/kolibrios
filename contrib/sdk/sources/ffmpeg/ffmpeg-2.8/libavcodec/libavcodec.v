LIBAVCODEC_$MAJOR {
        global: DllStartup;
                av*;
                #deprecated, remove after next bump
                audio_resample;
                audio_resample_close;
        local:  *;
};
