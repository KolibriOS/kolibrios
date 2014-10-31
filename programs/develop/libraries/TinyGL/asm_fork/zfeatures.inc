;
; If not debugging, assert does nothing.
;
;#define assert(x)	((void)0) /* debugging disabled */
;#include <assert.h> /* debugging enabled */

; It is possible to enable/disable (compile time) features in this
; header file.

;TGL_FEATURE_ARRAYS         equ 1
;TGL_FEATURE_DISPLAYLISTS   equ 1
;TGL_FEATURE_POLYGON_OFFSET equ 1

;
; Matrix of internal and external pixel formats supported. 'Y' means
; supported.
; 
;           External  8    16    24    32
; Internal 
;  15                 .     .     .     .
;  16                 Y     Y     Y     Y
;  24                 .     Y     Y     .
;  32                 .     Y     .     Y
; 
;
; 15 bpp does not work yet (although it is easy to add it - ask me if
; you need it).
; 
; Internal pixel format: see TGL_FEATURE_RENDER_BITS
; External pixel format: see TGL_FEATURE_xxx_BITS 
;

; enable various convertion code from internal pixel format (usually
; 16 bits per pixel) to any external format */
;TGL_FEATURE_16_BITS    equ 1
;TGL_FEATURE_8_BITS     equ 1
TGL_FEATURE_24_BITS    equ 1
;TGL_FEATURE_32_BITS    equ 1

;TGL_FEATURE_RENDER_BITS equ 15
;TGL_FEATURE_RENDER_BITS equ 16
TGL_FEATURE_RENDER_BITS equ 24
;TGL_FEATURE_RENDER_BITS equ 32
