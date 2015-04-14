;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  Include file for apps which want to use FTDI device         ;;
;;                                                              ;;
;;         GNU GENERAL PUBLIC LICENSE                           ;;
;;          Version 2, June 1991                                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;Bitmode constants
BITMODE_RESET  = 0x00       ;Switch off bitbang mode, back to regular serial FIFO
BITMODE_BITBANG= 0x01       ;Classical asynchronous bitbang mode, introduced with B-type chips */
BITMODE_MPSSE  = 0x02       ;MPSSE mode, available on 2232x chips */
BITMODE_SYNCBB = 0x04       ;Synchronous bitbang mode, available on 2232x and R-type chips  */
BITMODE_MCU    = 0x08       ;MCU Host Bus Emulation mode, available on 2232x chips */
    ;CPU-style fifo mode gets set via EEPROM (cuurenntly EEPROM burn not supported in KolibriOS)
BITMODE_OPTO   = 0x10       ;Fast Opto-Isolated Serial Interface Mode, available on 2232x chips  */
BITMODE_CBUS   = 0x20       ;Bitbang on CBUS pins of R-type chips, configure in EEPROM before */
BITMODE_SYNCFF = 0x40       ;Single Channel Synchronous FIFO mode, available on 2232H chips */
BITMODE_FT1284 = 0x80       ;FT1284 mode, available on 232H chips

;Line property constants
  ;parity
NONE        = 0 shl 8
ODD         = 1 shl 8
EVEN        = 2 shl 8
MARK        = 3 shl 8
SPACE       = 4 shl 8
  ;stop bits
STOP_BIT_1  = 0 shl 11
STOP_BIT_15 = 1 shl 11
STOP_BIT_2  = 2 shl 11
  ;break type
BREAK_OFF   = 0 shl 14
BREAK_ON    = 1 shl 14
  ;number of bits has to be at lower byte
  
;Flow control
SIO_DISABLE_FLOW_CTRL = 0x0
SIO_RTS_CTS_HS = (0x1 shl 8)
SIO_DTR_DSR_HS = (0x2 shl 8)
SIO_XON_XOFF_HS= (0x4 shl 8)

;Chip types
TYPE_AM     = 0
TYPE_BM     = 1
TYPE_2232C  = 2
TYPE_R      = 3 
TYPE_2232H  = 4
TYPE_4232H  = 5
TYPE_232H   = 6
TYPE_230X   = 7

;IO codes
;Get list of detected ftdi chips
FTDI_GET_LIST = 1
;Acquire lock on ftdi device
;[input+8] - none
;[output] - PID device is locked by
FTDI_LOCK     = 2
;Unlock ftdi device
;[input+8] - none
;[output] - 0 if unclock success, overwise PID device is locked by
FTDI_UNLOCK   = 3
;[output] - chunksize
FTDI_GET_WRITE_CHUNKSIZE = 4
;[output] - chunksize
FTDI_GET_READ_CHUNKSIZE  = 5
;[input+8] - chunksize
FTDI_SET_WRITE_CHUNKSIZE = 6
;[input+8] - chunksize
FTDI_SET_READ_CHUNKSIZE  = 7
;Writes data in chunks to the chip
;[input+8] - size of data, [input+12] - beginning of data
FTDI_WRITE_DATA          = 8
;Reads data in chunks from the chip
;[input+8] - size of data, [output] - red data
FTDI_READ_DATA           = 9
;Sets the chip baud rate
;[input+8] - baudrate
FTDI_SET_BAUDRATE        = 10
;Enable/disable bitbang modes
;[input+8] - 0xnnnnXXYY, where XX-bitmask, YY-bitmode
FTDI_SET_BITMODE         = 11
;Set rts line high
;[input+8] - none
FTDI_SET_RTS_HIGH        = 12
;Set rts line low
;[input+8] - none
FTDI_SET_RTS_LOW         = 13
;Set dtr line high
;[input+8] - none
FTDI_SET_DTR_HIGH        = 14
;Set dtr line low
;[input+8] - none
FTDI_SET_DTR_LOW         = 15
;Resets the ftdi device
;[input+8] - none
FTDI_USB_RESET           = 16
;Set flowcontrol for ftdi chip
;dword[input+8] - one of flow control constants
FTDI_SET_FLOW_CONTROL    = 17
;Set the special event character
;[input+8] - 0xnnnnXXYY, where XX-1/0(enable/disable), YY-event char
FTDI_SET_EVENT_CHAR      = 18
;Set error character
;[input+8] - 0xnnnnXXYY, where XX-1/0(enable/disable), YY-error char
FTDI_SET_ERROR_CHAR      = 19
;FTDI chip keeps data in the internal buffer for a specific
;amount of time if the buffer is not full yet to decrease
;load on the usb bus
;[input+8] - 0xnnnnXXXX, where XX-latency
FTDI_SET_LATENCY_TIMER   = 20
;Get latency timer
;[output] - latency
FTDI_GET_LATENCY_TIMER   = 21
;Directly read pin state, circumventing the read buffer. Useful for bitbang mode
;[output] - pins' values
FTDI_READ_PINS           = 22
;This function allows the retrieve the two status bytes of the device.
;The device sends these bytes also as a header for each read access
;where they are discarded by ftdi_read_data(). The chip generates
;the two stripped status bytes in the absence of data every 40 ms
;
; Layout of the first byte:
; - B0..B3 - must be 0
; - B4       Clear to send (CTS)
             ; 0 = inactive
             ; 1 = active
; - B5       Data set ready (DTS)
             ; 0 = inactive
             ; 1 = active
; - B6       Ring indicator (RI)
             ; 0 = inactive
             ; 1 = active
; - B7       Receive line signal detect (RLSD)
             ; 0 = inactive
             ; 1 = active

; Layout of the second byte:
; - B0       Data ready (DR)
; - B1       Overrun error (OE)
; - B2       Parity error (PE)
; - B3       Framing error (FE)
; - B4       Break interrupt (BI)
; - B5       Transmitter holding register (THRE)
; - B6       Transmitter empty (TEMT)
; - B7       Error in RCVR FIFO
;[output] - poll modem status
FTDI_POLL_MODEM_STATUS   = 23
;Set (RS232) line characteristics
;dword[input+8] - line property constants combination
FTDI_SET_LINE_PROPERTY   = 24
;Clears the read buffer on the chip
;[input+8] - none
FTDI_PURGE_RX_BUFFER     = 25
;Clears the write buffer on the chip
;[input+8] - none
FTDI_PURGE_RX_BUFFER     = 26

;Structures
  ;list layout from FTDI_GET_LIST
struct dev_list
size        dd  ?
nodes       rd  size*sizeof.node
ends

struct node
islocked    dd ? ; Will be 'LCKD', if dev is already locked; or 'NLKD', if dev is free
chip_type   dd ? ; FTDI chip type (see Chip types constants)
dev_handle  dd ? ; Device handle, value the same as in in_buf_map.dev_handle
ends

  ;input buffer map for requests above FTDI_GET_LIST
struc in_buf_map
pid         dd ? ; Should be PID of application using this file
dev_handle  dd ? ; Should be value acquired from one of dev_list's nodes
ends
