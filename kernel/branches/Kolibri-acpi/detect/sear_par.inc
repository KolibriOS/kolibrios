;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;****************************************************
;     поиск логических дисков на обнаруженных HDD
;     и занесение данных в область таблицы
;     автор Mario79
;****************************************************
        mov     [transfer_adress], DRIVE_DATA+0xa
 search_partitions_ide0:
        test    [DRIVE_DATA+1], byte 0x40
        jz      search_partitions_ide1
        mov     [hdbase], 0x1f0
        mov     [hdid], 0x0
        mov     [hdpos], 1
        mov     [known_part], 1
 search_partitions_ide0_1:
        call    set_PARTITION_variables
        test    [problem_partition], 2
        jnz     search_partitions_ide1  ; not found part
        test    [problem_partition], 1
        jnz     @F                      ; not found known_part
      ;cmp   [problem_partition],0
      ;jne   search_partitions_ide1
        inc     byte [DRIVE_DATA+2]
        call    partition_data_transfer
        add     [transfer_adress], 100
    @@:
        inc     [known_part]
        jmp     search_partitions_ide0_1

 search_partitions_ide1:
        test    [DRIVE_DATA+1], byte 0x10
        jz      search_partitions_ide2
        mov     [hdbase], 0x1f0
        mov     [hdid], 0x10
        mov     [hdpos], 2
        mov     [known_part], 1
 search_partitions_ide1_1:
        call    set_PARTITION_variables
        test    [problem_partition], 2
        jnz     search_partitions_ide2
        test    [problem_partition], 1
        jnz     @F
      ;cmp   [problem_partition],0
      ;jne   search_partitions_ide2
        inc     byte [DRIVE_DATA+3]
        call    partition_data_transfer
        add     [transfer_adress], 100
    @@:
        inc     [known_part]
        jmp     search_partitions_ide1_1

 search_partitions_ide2:
        test    [DRIVE_DATA+1], byte 0x4
        jz      search_partitions_ide3
        mov     [hdbase], 0x170
        mov     [hdid], 0x0
        mov     [hdpos], 3
        mov     [known_part], 1
 search_partitions_ide2_1:
        call    set_PARTITION_variables
        test    [problem_partition], 2
        jnz     search_partitions_ide3
        test    [problem_partition], 1
        jnz     @F
      ;cmp   [problem_partition],0
      ;jne   search_partitions_ide3
        inc     byte [DRIVE_DATA+4]
        call    partition_data_transfer
        add     [transfer_adress], 100
    @@:
        inc     [known_part]
        jmp     search_partitions_ide2_1

 search_partitions_ide3:
        test    [DRIVE_DATA+1], byte 0x1
        jz      end_search_partitions_ide
        mov     [hdbase], 0x170
        mov     [hdid], 0x10
        mov     [hdpos], 4
        mov     [known_part], 1
 search_partitions_ide3_1:
        call    set_PARTITION_variables
        test    [problem_partition], 2
        jnz     end_search_partitions_ide
        test    [problem_partition], 1
        jnz     @F
      ;cmp   [problem_partition],0
      ;jne   end_search_partitions_ide
        inc     byte [DRIVE_DATA+5]
        call    partition_data_transfer
        add     [transfer_adress], 100
    @@:
        inc     [known_part]
        jmp     search_partitions_ide3_1

end_search_partitions_ide:
        mov     [hdpos], 80h
        mov     ecx, [NumBiosDisks]
        test    ecx, ecx
        jz      end_search_partitions
start_search_partitions_bd:
        push    ecx
        mov     eax, [hdpos]
        and     [BiosDiskPartitions+(eax-80h)*4], 0
        mov     [known_part], 1
search_partitions_bd:
        call    set_PARTITION_variables
        test    [problem_partition], 2
        jnz     end_search_partitions_bd
        test    [problem_partition], 1
        jnz     @F
        ;cmp     [problem_partition], 0
        ;jne     end_search_partitions_bd
        mov     eax, [hdpos]
        inc     [BiosDiskPartitions+(eax-80h)*4]
        call    partition_data_transfer
        add     [transfer_adress], 100
    @@:
        inc     [known_part]
        jmp     search_partitions_bd
end_search_partitions_bd:
        pop     ecx
        inc     [hdpos]
        loop    start_search_partitions_bd
        jmp     end_search_partitions

problem_partition db 0  ; used for partitions search

include  '../fs/part_set.inc'

partition_data_transfer:
        mov     edi, [transfer_adress]
        mov     esi, PARTITION_START            ;start of file_system_data
        mov     ecx, (file_system_data_size+3)/4
        rep movsd
        ret
uglobal
transfer_adress dd 0
endg
partition_data_transfer_1:
;     cli
        push    edi
        mov     edi, PARTITION_START
        mov     esi, [transfer_adress]
        mov     ecx, (file_system_data_size+3)/4
        rep movsd
        pop     edi
;     sti
        ret

 end_search_partitions:

