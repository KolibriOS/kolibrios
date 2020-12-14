/*
   generated header with missing types for the
   jbig2dec program and library. include this
   after config.h, within the HAVE_CONFIG_H
   ifdef
*/

#ifndef HAVE_STDINT_H
#  ifdef JBIG2_REPLACE_STDINT_H
#   include <no_replacement_found>
#  else
    typedef unsigned int uint32_t;
    typedef unsigned short uint16_t;
    typedef unsigned char uint8_t;
    typedef signed int int32_t;
    typedef signed short int16_t;
    typedef signed char int8_t;
#  endif /* JBIG2_REPLACE_STDINT */
#endif /* HAVE_STDINT_H */
