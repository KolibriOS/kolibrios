; ================================= Import =================================== ;

library libio,          'libio.obj',    \
        libimg,         'libimg.obj'

import  libio,                          \
        file_size,      'file_size',    \
        file_open,      'file_open',    \
        file_read,      'file_read',    \
        file_close,     'file_close'

import  libimg,                         \
        img_decode,     'img_decode',   \
        img_convert,    'img_convert',  \
        img_scale,      'img_scale',    \
        img_flip,       'img_flip',     \
        img_rotate,     'img_rotate',   \
        img_to_rgb,     'img_to_rgb',   \
        img_destroy,    'img_destroy'