;  uFMOD API reference (NORMAL/UNSAFE mode)
;  ====================================================================

; NOTE: All functions follow the cdecl calling convention!
;    _uFMOD_LoadSong
;    _uFMOD_WaveOut
;    _uFMOD_StopSong
;    _uFMOD_Jump2Pattern
;    _uFMOD_Pause
;    _uFMOD_Resume
;    _uFMOD_GetStats
;    _uFMOD_GetRowOrder
;    _uFMOD_GetTime
;    _uFMOD_GetTitle
;    _uFMOD_SetVolume

;  ====================================================================
;  HANDLE _uFMOD_LoadSong(
;     void *lpXM,
;     void *param,
;     int fdwSong
;  )
;  ---
;  Description:
;  ---
;     Loads the given XM song and starts playing it as soon as you
;     call _uFMOD_WaveOut for the first time. Playback won't begin
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
;                    param is the image size. Once, _uFMOD_LoadSong
;                    returns, it's safe to free/discard the memory
;                    buffer.
;       XM_NOLOOP    An XM track plays repeatedly by default. Specify
;                    this flag to play it only once.
;       XM_SUSPENDED The XM track is loaded in a suspended state,
;                    and will not play until the _uFMOD_Resume function
;                    is called. This is useful for preloading a song
;                    or testing an XM track for validity.
;  ---
;  Return Values:
;  ---
;     On success, returns the handle of the Infinity Sound driver.
;     Returns 0 on failure.

;  ====================================================================
;  int _uFMOD_WaveOut(void)
;  ---
;  Description:
;  ---
;     Updates the internal playback buffer.
;  ---
;  Remarks:
;  ---
;     This function should be called from the same thread
;     _uFMOD_LoadSong was previously called. Playback doesn't actually
;     begin when calling _uFMOD_LoadSong, but when calling _uFMOD_WaveOut
;     after a successful _uFMOD_LoadSong call. Afterwards, you should
;     call _uFMOD_WaveOut repeatedly at least once every 250 ms to
;     prevent "buffer underruns".
;     _uFMOD_WaveOut is a non-blocking function. The accuracy of the
;     InfoAPI functions (_uFMOD_GetStats, _uFMOD_GetRowOrder and
;     _uFMOD_GetTime) depends on the periodicity of this function being
;     invoked.
;  ---
;  Return Values:
;  ---
;     Returns non zero on error.

;  ====================================================================
;  void _uFMOD_StopSong(void)
;  ---
;  Description:
;  ---
;     Stops the currently playing song, freeing the associated
;     resources.
;  ---
;  Remarks:
;  ---
;     Does nothing if no song is playing at the time the call is made.

;  ====================================================================
;  void _uFMOD_Jump2Pattern(
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
;     commands in all patterns intended to be used as _uFMOD_Jump2Pattern
;     targets.
;     if the pattern index lays outside of the bounds of the pattern order
;     table, calling this function jumps to pattern 0, effectively
;     rewinding playback.

;  ====================================================================
;  void _uFMOD_Pause(void)
;  ---
;  Description:
;  ---
;     Pauses the currently playing song, if any.
;  ---
;  Remarks:
;  ---
;     While paused you can still control the volume (_uFMOD_SetVolume) and
;     the pattern order (_uFMOD_Jump2Pattern). The RMS volume coefficients
;     (_uFMOD_GetStats) will go down to 0 and the progress tracker
;     (_uFMOD_GetTime) will "freeze" while the song is paused.
;     _uFMOD_Pause doesn't perform the request immediately. Instead, it
;     signals to pause when playback reaches next chunk of data.
;     This way, _uFMOD_Pause performs asynchronously and returns very fast.
;     It is not cumulative. So, calling _uFMOD_Pause many times in a row
;     has the same effect as calling it once.
;     You shouldn't stop calling _uFMOD_WaveOut while the song is paused!

;  ====================================================================
;  void _uFMOD_Resume(void)
;  ---
;  Description:
;  ---
;     Resumes the currently paused song, if any.
;  ---
;  Remarks:
;  ---
;     _uFMOD_Resume doesn't perform the request immediately. Instead, it
;     signals to resume when _uFMOD_WaveOut is called again. _uFMOD_Resume
;     is not cumulative. So, calling it many times in a row has the same
;     effect as calling it once.

;  ====================================================================
;  unsigned int _uFMOD_GetStats(void)
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
;     _uFMOD_GetStats() as often as you like, but take in mind that uFMOD
;     updates both channel RMS volumes at the same rate _uFMOD_WaveOut
;     function is called. In other words, you should call _uFMOD_WaveOut
;     more often to increase the accuracy of _uFMOD_GetStats.

;  ====================================================================
;  unsigned int _uFMOD_GetRowOrder(void)
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
;     row and order values at the same rate _uFMOD_WaveOut function is
;     called. In other words, you should call _uFMOD_WaveOut more often
;     to increase the accuracy of _uFMOD_GetRowOrder.

;  ====================================================================
;  unsigned int _uFMOD_GetTime(void)
;  ---
;  Description:
;  ---
;     Returns the time in milliseconds since the song was started.
;  ---
;  Remarks:
;  ---
;     This function is useful for synchronizing purposes. Multimedia
;     applications can use _uFMOD_GetTime to synchronize GFX to sound,
;     for example. An XM player can use this function to update a progress
;     meter.

;  ====================================================================
;  unsigned char* _uFMOD_GetTitle(void)
;  ---
;  Description:
;  ---
;     Returns the current song's title.
;  ---
;  Remarks:
;  ---
;     Not every song has a title, so be prepared to get an empty string.

;  ====================================================================
;  void _uFMOD_SetVolume(
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
;     _uFMOD_LoadSong calls. You can set the desired volume level before
;     actually starting to play a song.
;     You can use Infinity Sound API to control the L and R channels volumes
;     separately. It also has a wider range than _uFMOD_SetVolume, sometimes
;     allowing to amplify the sound volume as well, as opposed to
;     _uFMOD_SetVolume only being able to attenuate it.

XM_MEMORY         equ 1
XM_FILE           equ 2
XM_NOLOOP         equ 8
XM_SUSPENDED      equ 16
uFMOD_MIN_VOL     equ 0
uFMOD_MAX_VOL     equ 25
uFMOD_DEFAULT_VOL equ 25
