; Now being rewritten from https://github.com/nayuki/QR-Code-generator/blob/master/c/qrcodegen.c

; __DEBUG__       = 1
; __DEBUG_LEVEL__ = DBG_ALL
; DBG_ALL       = 0  ; all messages
; DBG_INFO      = 1  ; info and errors
; DBG_ERR       = 2  ; only errors

include '../../proc32.inc'
include '../../struct.inc'
include '../../if.inc'
; include '../../debug-fdo.inc'

; The error correction level in a QR Code symbol.
qrcodegen_Ecc_LOW      = 0 ; The QR Code can tolerate about  7% erroneous codewords
qrcodegen_Ecc_MEDIUM   = 1 ; The QR Code can tolerate about 15% erroneous codewords
qrcodegen_Ecc_QUARTILE = 2 ; The QR Code can tolerate about 25% erroneous codewords
qrcodegen_Ecc_HIGH     = 3 ; The QR Code can tolerate about 30% erroneous codewords

; The mask pattern used in a QR Code symbol.
;
; A special value to tell the QR Code encoder to
; automatically select an appropriate mask pattern:
qrcodegen_Mask_AUTO = -1
; The eight actual mask patterns:
qrcodegen_Mask_0    = 0
qrcodegen_Mask_1    = 1
qrcodegen_Mask_2    = 2
qrcodegen_Mask_3    = 3
qrcodegen_Mask_4    = 4
qrcodegen_Mask_5    = 5
qrcodegen_Mask_6    = 6
qrcodegen_Mask_7    = 7

; Describes how a segment's data bits are interpreted.
qrcodegen_Mode_NUMERIC      = 0x1
qrcodegen_Mode_ALPHANUMERIC = 0x2
qrcodegen_Mode_BYTE         = 0x4
qrcodegen_Mode_KANJI        = 0x8
qrcodegen_Mode_ECI          = 0x7

; A segment of character/binary/control data in a QR Code symbol.
; The mid-level way to create a segment is to take the payload data
; and call a factory function such as qrcodegen_makeNumeric().
; The low-level way to create a segment is to custom-make the bit buffer
; and initialize a qrcodegen_Segment struct with appropriate values.
; Even in the most favorable conditions, a QR Code can only hold 7089 characters of data.
; Any segment longer than this is meaningless for the purpose of generating QR Codes.
; Moreover, the maximum allowed bit length is 32767 because
; the largest QR Code (version 40) has 31329 modules.
struct qrcodegen_Segment
        ; The mode indicator of this segment.
        mode dd ? ; qrcodegen_Mode
        
        ; The length of this segment's unencoded data. Measured in characters for
        ; numeric/alphanumeric/kanji mode, bytes for byte mode, and 0 for ECI mode.
        ; Always zero or positive. Not the same as the data's bit length.
        numChars dd ?
        
        ; The data bits of this segment, packed in bitwise big endian.
        ; Can be null if the bit length is zero.
        data dd ?
        
        ; The number of valid data bits used in the buffer. Requires
        ; 0 <= bitLength <= 32767, and bitLength <= (capacity of data array) * 8.
        ; The character count (numChars) must agree with the mode and the bit buffer length.
        bitLength dd ?
ends

; The minimum version number supported in the QR Code Model 2 standard
qrcodegen_VERSION_MIN =  1  
; The maximum version number supported in the QR Code Model 2 standard
qrcodegen_VERSION_MAX = 40

; Calculates the number of bytes needed to store any QR Code up to and including the given version number,
; as a compile-time constant. For example, 'uint8_t buffer[qrcodegen_BUFFER_LEN_FOR_VERSION(25)];'
; can store any single QR Code from version 1 to 25 (inclusive). The result fits in an int (or int16).
; Requires qrcodegen_VERSION_MIN <= n <= qrcodegen_VERSION_MAX.
; TODO make this fasm macro:
; #define qrcodegen_BUFFER_LEN_FOR_VERSION(n)  ((((n) * 4 + 17) * ((n) * 4 + 17) + 7) / 8 + 1)

; The worst-case number of bytes needed to store one QR Code, up to and including
; version 40. This value equals 3918, which is just under 4 kilobytes.
; Use this more convenient value to avoid calculating tighter memory bounds for buffers.
; TODO make this fasm macro:
; #define qrcodegen_BUFFER_LEN_MAX  qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_VERSION_MAX)

INT16_MAX = 32767

; Sentinel value for use in only some functions
LENGTH_OVERFLOW  = -1

; Based on the table ECC_CODEWORDS_PER_BLOCK
qrcodegen_REED_SOLOMON_DEGREE_MAX = 30 

; For automatic mask pattern selection.
PENALTY_N1 =  3
PENALTY_N2 =  3
PENALTY_N3 = 40
PENALTY_N4 = 10


; CODE:

proc qrcodegen_encodeBinary, dataAndTemp, dataLen, qrcode, ecl, minVersion, maxVersion, mask, boostEcl
locals
        seg qrcodegen_Segment
endl
        mov     [seg + qrcodegen_Segment.mode], qrcodegen_Mode_BYTE
        stdcall calcSegmentBitLength, [seg + qrcodegen_Segment.mode], [dataLen]
        mov     [seg + qrcodegen_Segment.bitLength], eax
        .if [seg + qrcodegen_Segment.bitLength] = LENGTH_OVERFLOW
            mov byte [qrcode + 0], 0 ; Set size to invalid value for safety
            jmp .ret
        .endif
        mov     eax, [dataLen]
        mov     [seg + qrcodegen_Segment.numChars], eax
        mov     eax, [dataAndTemp]
        mov     [seg + qrcodegen_Segment.data], eax
        lea     eax, [seg]
        stdcall qrcodegen_encodeSegmentsAdvanced, eax, 1, [ecl], [minVersion], [maxVersion], [mask], [boostEcl], [dataAndTemp], [qrcode]
.ret:
        ret
endp



; Returns the number of data bits needed to represent a segment
; containing the given number of characters using the given mode. Notes:
; - Returns LENGTH_OVERFLOW on failure, i.e. numChars > INT16_MAX
;   or the number of needed bits exceeds INT16_MAX (i.e. 32767).
; - Otherwise, all valid results are in the range [0, INT16_MAX].
; - For byte mode, numChars measures the number of bytes, not Unicode code points.
; - For ECI mode, numChars must be 0, and the worst-case number of bits is returned.
;   An actual ECI segment can have shorter data. For non-ECI modes, the result is exact.

proc calcSegmentBitLength uses edx ebx, mode, numChars
        .if [numChars] > INT16_MAX
            mov eax, LENGTH_OVERFLOW
            jmp .ret
        .endif
        mov   eax, [numChars]
        .if [mode] = qrcodegen_Mode_NUMERIC
            ; eax = ceil(10/3 * n)
            xor edx, edx
            mov ebx, 10
            mul ebx
            add eax, 2
            mov ebx, 3
            div eax, ebx
        .elseif [mode] = qrcodegen_Mode_ALPHANUMERIC
            ; eax = ceil(11/2 * n)
            xor edx, edx
            mov ebx, 11
            mul eax, ebx
            inc eax
            mov ebx, 2
            div eax, ebx
        .elseif [mode] = qrcodegen_Mode_BYTE
            shl eax, 3  ; *8 
        .elseif [mode] = qrcodegen_Mode_KANJI
            xor edx, edx
            mov ebx, 13
            mul ebx
        .elseif [mode] = qrcodegen_Mode_ECI
            .if [numChars] = 0
                mov eax, 3*8
            .endif
        .else ;  Invalid argument
            mov eax, LENGTH_OVERFLOW
            jmp .ret
        .endif
        ; TODO  assert(result >= 0)
        .if eax > INT16_MAX
                mov eax, LENGTH_OVERFLOW
        .endif
.ret:
        ret
endp



; DATA:

; include_debug_strings

; The set of all legal characters in alphanumeric mode, where each character
; value maps to the index in the string. For checking text and encoding segments.
ALPHANUMERIC_CHARSET db "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:",0

; For generating error correction codes.
ECC_CODEWORDS_PER_BLOCK db \
 \ ; Version: (note that index 0 is for padding, and is set to an illegal value)
 \ ; 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40    Error correction level
    -1,  7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, \  ; Low
    -1, 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, \  ; Medium
    -1, 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, \  ; Quartile
    -1, 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30     ; High

; For generating error correction codes.
NUM_ERROR_CORRECTION_BLOCKS db \
 \ ; Version: (note that index 0 is for padding, and is set to an illegal value)
 \ ; 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40    Error correction level
    -1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25, \  ; Low
    -1, 1, 1, 1, 2, 2, 4, 4, 4, 5, 5,  5,  8,  9,  9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49, \  ; Medium
    -1, 1, 1, 2, 2, 4, 4, 6, 6, 8, 8,  8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68, \  ; Quartile
    -1, 1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81     ; High


