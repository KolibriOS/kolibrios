;*****************************************************************************;
;    Copyright (C) 2025, Mikhail Frolov aka Doczom . All rights reserved.     ;
;            Distributed under terms of the 3-Clause BSD License.             ;
;*****************************************************************************;
;               File of usb drivers
;       base = 0
; offset                | Size  | Name          | Description
;=======================|=======|===============|=============================
; base + 0              |  4    | ID_drv_table  | offset to dev_ven drv array
; base + 4              |  4    | Class_table   | offset to class drv array
;
;       n = 0 .. count driver with ID table
;       nt = count driver with ID table
; ID_drv_table + 0*(n-1)|  4    | ID_TABLE      | offset of base to table VID:PID
; ID_drv_table + 4*(n-1)|  4    | DRV_NAME      | offset of base to name the driver
; ID_drv_table + 8*nt   |  8    | __ZERO        | terminaror of list

;       k = 0 .. count driver on class code
;       kt = count driver on class code
; Class_table + 0*(k-1) |  1    | LENGTH_CLASS  | length of class code 1..3
; Class_table + 1*(k-1) |  1    | USB_CLASS     | main usb class code (M)
; Class_table + 2*(k-1) |  1    | USB_SUBCLASS  | USB subclass code or zero(V)
; Class_table + 2*(k-1) |  1    | USB_PROTOCOL  | USB protocol code or zero(V)
; Class_table + 4*(k-1) |  4    | DRV_NAME      | offset of base to name the driver
; Class_table + 8*kt    |  8    | __ZERO        | terminaror of list

;       i = 0 .. count VID:PID
;       it =  count VID:PID
; ID_TABLE + 0*(i-1)    |  2    | VID           | Vendor id
; ID_TABLE + 2*(i-1)    |  2    | PID           | Product id
; ID_TABLE + 4*it       |  4    | __ZERO        | terminaror of list


macro INIT_USBDRV_FILE {
        local ..id_list, ..class_list

        format binary as 'dat'
        use32
        org 0
                dd      ..id_list
                dd      ..class_list


        macro ID_DRV_TABLE \{
                dd 0, 0
        \}

        macro CLASS_TABLE \{
                dd 0, 0
        \}
        macro ID_TABLE \{
        \}
        macro DRV_NAME_LIST \{
        \}
        postpone \{
                 ..id_list:   ID_DRV_TABLE
                ..class_list: CLASS_TABLE
                ID_TABLE
                DRV_NAME_LIST
        \}
}

macro ADD_CLASS drv_name, class, subclass, protocol {

        local ..length, ..class_code, ..drv_name

        ..length = 3

        match =X, class \{
              err 'Class is mandatory argument'
        \}
        match =X, protocol \{
              ..length = 2
        \}
        match =X, subclass \{
              ..length = 1
        \}

        ..class_code = ..length or (class  shl 8)

        if  ..length = 3
            ..class_code = ..class_code or (protocol  shl 24)
        end if
        if  ..length = 2
            ..class_code = ..class_code or (subclass  shl 16)
        end if


        ; add in list

        macro DRV_NAME_LIST \{
                ..drv_name: db drv_name, 0
                DRV_NAME_LIST
        \}

        macro CLASS_TABLE \{
                dd      ..class_code, ..drv_name
                CLASS_TABLE
        \}
}

macro ADD_ID drv_name, [device_id] {
        common
        local ..drv_name, ..id_table

        macro ID_TABLE \{
                ID_TABLE
                ..id_table:
        \}

        reverse
        local vid_pid
                match VID:PID, device_id \{
                      vid_pid = (PID shl 16) + VID
                \}


        macro ID_TABLE \{
                ID_TABLE
                dd  vid_pid
        \}

        common
        macro ID_TABLE \{
                ID_TABLE
                dd  0
        \}
        macro DRV_NAME_LIST \{
                ..drv_name: db drv_name, 0
                DRV_NAME_LIST
        \}
        macro ID_DRV_TABLE \{
                dd ..id_table, ..drv_name
                ID_DRV_TABLE
        \}
}

; ADD ID driver Linux
macro ADD_IDL drv_name, [vendor_id, device_id] {
        common
        local ..drv_name, ..id_table

        macro ID_TABLE \{
                ID_TABLE
                ..id_table:
        \}

        reverse
        local vid_pid
                vid_pid = (device_id shl 16) + vendor_id

        macro ID_TABLE \{
                ID_TABLE
                dd  vid_pid
        \}

        common
        macro ID_TABLE \{
                ID_TABLE
                dd  0
        \}
        macro DRV_NAME_LIST \{
                ..drv_name: db drv_name, 0
                DRV_NAME_LIST
        \}
        macro ID_DRV_TABLE \{
                dd ..id_table, ..drv_name
                ID_DRV_TABLE
        \}
}


INIT_USBDRV_FILE

;ADD_CLASS       'usbcdc-ctrl'   , 0x02, X   , X
;ADD_CLASS       'usbimage'      , 0x06, 1   , 1
;ADD_CLASS       'usbcdc-data'   , 0x0A, X   , X
;ADD_CLASS       'ccid'          , 0x0B, X   , X
;ADD_CLASS       'uvd'           , 0x0E, X   , X
;ADD_CLASS       'uvd_2'         , 0x0E, 0x02, X
;ADD_CLASS       'usb_bluetooth' , 0xE0, 0x01, X
;ADD_CLASS       'usb_wifi'      , 0xE0, 0x02, X



ADD_ID  'usbftdi',\
        0x0403:0        ; Any FTDI device

;https://github.com/avrdudes/avrdude/blob/main/src/usbdevs.h#L51
;ADD_ID  'usbasp',\
;        0x16c0:0x05dc,\ ; VOTI Obdev's free shared PID
;        0x03e8:0xc7b4,\ ; ATMEL (unofficial) USBasp
;        0x16c0:0x092f   ; VOTI NIBObee PID

;https://github.com/WCHSoftGroup/ch341par_linux/blob/main/driver/ch34x_pis.c
;ADD_IDL 'ch341par',\
;        0x1a86, 0x5512,\ ; ch341a default
;        0x1a86, 0x55db,\ ; CH347T Mode1 SPI+IIC+UART
;        0x1a86, 0x55dd,\ ; CH347T Mode3 JTAG+UART
;        0x1a86, 0x55de,\ ; CH347F
;        0x1a86, 0x55e7   ; CH339W

;https://github.com/openbsd/src/blob/master/sys/dev/usb/uchcom.c
;https://github.com/WCHSoftGroup/ch341ser_linux/blob/main/driver/ch341.c
;ADD_IDL 'ch341ser',\
;        0x1a86, 0x7523,\ ; ch340 chip
;        0x1a86, 0x7522,\ ; ch340k chip
;        0x1a86, 0x5523,\ ; ch341 chip
;        0x1a86, 0xe523,\ ; ch330 chip
;        0x4348, 0x5523   ; ch340 custom chip




