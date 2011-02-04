/*
   uFMOD API reference (AC97SND mode)
   =========================================================

   NOTE: ufmod.obj should be rebuilt setting UF_MODE=AC97SND
   in order to make it usable in AC97SND player.

   The Infinity Sound driver handle should be available as
   a public symbol named hSound. It is so when using Serge's
   sound.lib.
*/

#ifdef __cplusplus
	extern "C" {
#endif

/* HANDLE uFMOD_LoadSong(char *lpXM);
   ---
   Description:
   ---
      Loads the given XM song and starts playing it as soon as you
      call uFMOD_WaveOut for the first time. It will stop any
      currently playing song before loading the new one. Heap should
      be initialized before calling this function!
   ---
   Parameters:
   ---
     lpXM
        Specifies the filename of the song to load.
   ---
   Return Values:
   ---
      On success, returns a non zero value. Returns 0 on failure.
*/
int __cdecl uFMOD_LoadSong(char*);

/* int uFMOD_WaveOut(SNDBUF hBuff)
   ---
   Description:
   ---
      Updates the internal playback buffer.
   ---
   Parameters:
   ---
     hBuff
        The Infinity Sound buffer to update.
   ---
   Remarks:
   ---
      Playback doesn't actually begin when calling uFMOD_LoadSong,
      but when calling uFMOD_WaveOut after a successful uFMOD_LoadSong
      call. Afterwards, you should call uFMOD_WaveOut repeatedly at
      least once every 250 ms to prevent "buffer underruns".
      uFMOD_WaveOut is a non-blocking function.
   ---
   Return Values:
   ---
      Returns non zero on error.
*/
int __cdecl uFMOD_WaveOut(unsigned int);

/* void uFMOD_StopSong(void)
   ---
   Description:
   ---
      Stops the currently playing song, freeing the associated
      resources.
   ---
   Remarks:
   ---
      Does nothing if no song is playing at the time the call is made.
*/
void __cdecl uFMOD_StopSong();

/* unsigned char* _uFMOD_GetTitle(void)
   ---
   Description:
   ---
      Returns the current song's title.
   ---
   Remarks:
   ---
      Not every song has a title, so be prepared to get an empty string.
*/
unsigned char* __cdecl uFMOD_GetTitle();

#ifdef __cplusplus
	}
#endif
