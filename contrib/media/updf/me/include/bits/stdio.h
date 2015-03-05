#ifndef __BITS_STDIO_H
#define __BITS_STDIO_H

struct file_stream_ops {
 int (* s_putc)(struct __FILE *,int);
 int (* s_getc)(struct __FILE *,int *);
 int (* s_read)(struct __FILE *,void *,int);
 int (* s_write)(struct __FILE *,void *,int);
 int (* s_seek)(struct __FILE *,int,int);
 int (* s_flush)(struct __FILE *);
};

#define STM_OP(x,n) \
    (x)->std_ops->s_##n

#endif
