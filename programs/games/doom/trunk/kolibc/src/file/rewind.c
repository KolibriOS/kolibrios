
#include "kolibc.h"

void rewind(FILE* file)
{
        file->filepos=0;
        file->stream=file->buffer;
        file->strpos=0;
        file->remain=8192;
        
        fill_buff(file);
}
