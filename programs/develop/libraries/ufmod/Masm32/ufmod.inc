;  uFMOD header file
;  Target OS: KolibriOS
;  Compiler:  MASM32

;  HANDLE uFMOD_LoadSong(
;     void *lpXM,
;     void *param,
;     int fdwSong
;  )
;  ---
;  Description:
;  ---
;     Loads the given XM song and starts playing it as soon as you
;     call uFMOD_WaveOut for the first time. Playback won't begin
;     if XM_SUSPENDED flag is specified. It will stop any currently
;     playing song before loading the new one.
;  ---
;  Parameters:
;  ---
;    lpXM
;       Specifies the song to load. If this parameter is 0, any
;       currently playing song is stopped. In such a case, function
;       does not return a meaningful value. fdwSong parameter
;       determines whether this value is interpreted as a filename
;       or as a pointer to an image of the song in memory.
;    param
;       If XM_MEMORY is specified, this parameter should be the size
;       of the image of the song in memory.
;       If XM_FILE is specified, this parameter is ignored.
;    fdwSong
;       Flags for playing the song. The following values are defined:
;       XM_FILE      lpXM points to filename. param is ignored.
;       XM_MEMORY    lpXM points to an image of a song in memory.
;                    param is the image size. Once, uFMOD_LoadSong
;                    returns, it's safe to free/discard the memory
;                    buffer.
;       XM_NOLOOP    An XM track plays repeatedly by default. Specify
;                    this flag to play it only once.
;       XM_SUSPENDED The XM track is loaded in a suspended state,
;                    and will not play until the uFMOD_Resume function
;                    is called. This is useful for preloading a song
;                    or testing an XM track for validity.
;  ---
;  Return Values:
;  ---
;     On success, returns the handle of the Infinity Sound driver.
;     Returns 0 on failure.
uFMOD_LoadSong PROTO C :DWORD,:DWORD,:DWORD

;  int uFMOD_WaveOut(void)
;  ---
;  Description:
;  ---
;     Updates the internal playback buffer.
;  ---
;  Remarks:
;  ---
;     This function should be called from the same thread
;     uFMOD_LoadSong was previously called. Playback doesn't actually
;     begin when calling uFMOD_LoadSong, but when calling uFMOD_WaveOut
;     after a successful uFMOD_LoadSong call. Afterwards, you should
;     call uFMOD_WaveOut repeatedly at least once every 250 ms to
;     prevent "buffer underruns".
;     uFMOD_WaveOut is a non-blocking function. The accuracy of the
;     InfoAPI functions (uFMOD_GetStats, uFMOD_GetRowOrder and
;     uFMOD_GetTime) depends on the periodicity of this function being
;     invoked.
;  ---
;  Return Values:
;  ---
;     Returns non zero on error.
uFMOD_WaveOut PROTO C

;  void uFMOD_StopSong(void)
;  ---
;  Description:
;  ---
;     Stops the currently playing song, freeing the associated
;     resources.
;  ---
;  Remarks:
;  ---
;     Does nothing if no song is playing at the time the call is made.
uFMOD_StopSong PROTO C

;  void uFMOD_Jump2Pattern(
;     unsigned int pat
;  )
;  ---
;  Description:
;  ---
;     Jumps to the specified pattern index.
;  ---
;  Parameters:
;  ---
;     pat
;        Next zero based pattern index.
;  ---
;  Remarks:
;  ---
;     uFMOD doesn't automatically perform Note Off effects before jumping
;     to the target pattern. In other words, the original pattern will
;     remain in the mixer until it fades out. You can use this feature to
;     your advantage. If you don't like it, just insert leading Note Off
;     commands in all patterns intended to be used as uFMOD_Jump2Pattern
;     targets.
;     if the pattern index lays outside of the bounds of the pattern order
;     table, calling this function jumps to pattern 0, effectively
;     rewinding playback.
uFMOD_Jump2Pattern PROTO C :DWORD

;  void uFMOD_Pause(void)
;  ---
;  Description:
;  ---
;     Pauses the currently playing song, if any.
;  ---
;  Remarks:
;  ---
;     While paused you can still control the volume (uFMOD_SetVolume) and
;     the pattern order (uFMOD_Jump2Pattern). The RMS volume coefficients
;     (uFMOD_GetStats) will go down to 0 and the progress tracker
;     (uFMOD_GetTime) will "freeze" while the song is paused.
;     uFMOD_Pause doesn't perform the request immediately. Instead, it
;     signals to pause when playback reaches next chunk of data.
;     This way, uFMOD_Pause performs asynchronously and returns very fast.
;     It is not cumulative. So, calling uFMOD_Pause many times in a row
;     has the same effect as calling it once.
;     You shouldn't stop calling uFMOD_WaveOut while the song is paused!
uFMOD_Pause PROTO C

;  void uFMOD_Resume(void)
;  ---
;  Description:
;  ---
;     Resumes the currently paused song, if any.
;  ---
;  Remarks:
;  ---
;     uFMOD_Resume doesn't perform the request immediately. Instead, it
;     signals to resume when uFMOD_WaveOut is called again. uFMOD_Resume
;     is not cumulative. So, calling it many times in a row has the same
;     effect as calling it once.
uFMOD_Resume PROTO C

;  unsigned int uFMOD_GetStats(void)
;  ---
;  Description:
;  ---
;     Returns the current RMS volume coefficients in (L)eft and (R)ight
;     channels.
;        low-order word: RMS volume in R channel
;        hi-order word:  RMS volume in L channel
;     Range from 0 (silence) to $7FFF (maximum) on each channel.
;  ---
;  Remarks:
;  ---
;     This function is useful for updating a VU meter. It's recommended
;     to rescale the output to log10 (decibels or dB for short), because
;     human ears track volume changes in a dB scale. You may call
;     uFMOD_GetStats() as often as you like, but take in mind that uFMOD
;     updates both channel RMS volumes at the same rate uFMOD_WaveOut
;     function is called. In other words, you should call uFMOD_WaveOut
;     more often to increase the accuracy of uFMOD_GetStats.
uFMOD_GetStats PROTO C

;  unsigned int uFMOD_GetRowOrder(void)
;  ---
;  Description:
;  ---
;     Returns the currently playing row and order.
;        low-order word: row
;        hi-order word:  order
;  ---
;  Remarks:
;  ---
;     This function is useful for synchronization. uFMOD updates both
;     row and order values at the same rate uFMOD_WaveOut function is
;     called. In other words, you should call uFMOD_WaveOut more often
;     to increase the accuracy of uFMOD_GetRowOrder.
uFMOD_GetRowOrder PROTO C

;  unsigned int uFMOD_GetTime(void)
;  ---
;  Description:
;  ---
;     Returns the time in milliseconds since the song was started.
;  ---
;  Remarks:
;  ---
;     This function is useful for synchronizing purposes. Multimedia
;     applications can use uFMOD_GetTime to synchronize GFX to sound,
;     for example. An XM player can use this function to update a progress
;     meter.
uFMOD_GetTime PROTO C

;  unsigned char* uFMOD_GetTitle(void)
;  ---
;  Description:
;  ---
;     Returns the current song's title.
;  ---
;  Remarks:
;  ---
;     Not every song has a title, so be prepared to get an empty string.
uFMOD_GetTitle PROTO C

;  void uFMOD_SetVolume(
;     unsigned int vol
;  )
;  ---
;  Description:
;  ---
;     Sets the global volume. The volume scale is linear.
;  ---
;  Parameters:
;  ---
;     vol
;        New volume. Range: from uFMOD_MIN_VOL (muting) to uFMOD_MAX_VOL
;        (maximum volume). Any value above uFMOD_MAX_VOL maps to maximum
;        volume.
;  ---
;  Remarks:
;  ---
;     uFMOD internally converts the given values to a logarithmic scale (dB).
;     Maximum volume is set by default. The volume value is preserved across
;     uFMOD_LoadSong calls. You can set the desired volume level before
;     actually starting to play a song.
;     You can use Infinity Sound API to control the L and R channels volumes
;     separately. It also has a wider range than uFMOD_SetVolume, sometimes
;     allowing to amplify the sound volume as well, as opposed to
;     uFMOD_SetVolume only being able to attenuate it.
uFMOD_SetVolume PROTO C :DWORD

XM_MEMORY         EQU 1
XM_FILE           EQU 2
XM_NOLOOP         EQU 8
XM_SUSPENDED      EQU 16
uFMOD_MIN_VOL     EQU 0
uFMOD_MAX_VOL     EQU 25
uFMOD_DEFAULT_VOL EQU 25