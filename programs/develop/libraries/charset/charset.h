
int version();

// 866 -> 1251
void __stdcall dos2win (const char text_in[], char text_out[]);

// 1251 -> 866
void __stdcall win2dos (const char text_in[], char text_out[]);

// koi8-r -> 866
void __stdcall koi2dos (const char text_in[], char text_out[]);
