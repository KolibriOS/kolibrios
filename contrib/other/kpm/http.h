#ifndef __HTTP_H__
#define __HTTP_H__

#define FLAG_GOT_ALL_DATA       (1 << 2)

#define FLAG_STREAM             (1 << 9)
#define FLAG_REUSE_BUFFER       (1 << 10)


typedef struct
{
    int   socket;               // socket on which the actual transfer happens
    int   flags;                // flags, reflects status of the transfer using bitflags
    int   write_ptr;            // internal use only (where to write new data in buffer)
    int   buffer_length;        // internal use only (number of available bytes in buffer)
    int   chunk_ptr;            // internal use only (where the next chunk begins)
    int   timestamp;            // internal use only (when last data was received)

    int   status;               // HTTP status
    int   header_length;        // length of HTTP header
    void *content_ptr;          // ptr to content
    int   content_length;       // total length of HTTP content
    int   content_received;     // number of currently received content bytes
}http_t;

int http_init();
int http_load(char *buf, const char *path);

http_t* __stdcall http_get(const char *url, http_t *conn, int flags, const char *header);
int     __stdcall http_receive(http_t *conn);
void    __stdcall http_free(http_t *conn);

static inline int http_receive_with_retry(http_t *http, int retry_count)
{
    int err;

    do
    {
        err = http_receive(http);
        if(err)
            wait_for_event(1);

    }while(err && --retry_count);

    return err;
}

void     __stdcall con_init(unsigned w_w, unsigned w_h, unsigned s_w, unsigned s_h, const char* t);
void     __stdcall con_exit(char bCloseWindow);
unsigned __stdcall con_get_flags(void);
unsigned __stdcall con_set_flags(unsigned new_flags);
void     __stdcall con_cls(void);
void     __stdcall con_write_asciiz(const char* string);

char *make_url(const char *name);
char *make_tmp_path(const char *path);
char *make_cache_path(const char *path);

#endif /* __HTTP_H__ */
