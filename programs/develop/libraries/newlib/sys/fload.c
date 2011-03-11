
#include <sys/types.h>
#include <stdint.h>
#include <sys/kos_io.h>

void *load_file(const char *path, size_t *len)
{
    fileinfo_t   info;
    size_t       bytes;
    void        *file = NULL;

    if( len) *len = 0;


    if( !get_fileinfo(path, &info) )
    {

        file = user_alloc( info.size );
        read_file(path, file, 0, info.size, &bytes );
        if( bytes == info.size )
        {
            if ( *(uint32_t*)file == 0x4B43504B )
            {
                void *tmp = NULL;
                info.size = ((size_t*)file)[1];
                tmp = user_alloc(info.size);
                unpack(file, tmp);
                user_free(file);
                file = tmp;
            }
            if(len) *len = info.size;
        };
    };
    return file;
};


