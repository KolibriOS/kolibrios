;****************************************************
;     поиск логических дисков на обнаруженных HDD
;     и занесение данных в область таблицы
;     автор Mario79
;****************************************************
      mov   [transfer_adress],0x4000a
 search_partitions_ide0:
      test  [0x40001],byte 0x40
      jz   search_partitions_ide1
        mov   [hdbase],0x1f0
        mov   [hdid],0x0
        mov   [hdpos],1
      mov  [fat32part],1
 search_partitions_ide0_1:
      call  set_FAT32_variables
      cmp   [problem_partition],0
      jne   search_partitions_ide1
      inc   byte [0x40002]
      call  partition_data_transfer
      add   [transfer_adress],100
      inc   [fat32part]
      jmp   search_partitions_ide0_1

 search_partitions_ide1:
      test  [0x40001],byte 0x10
      jz   search_partitions_ide2
        mov   [hdbase],0x1f0
        mov   [hdid],0x10
        mov   [hdpos],2
      mov  [fat32part],1
 search_partitions_ide1_1:
      call  set_FAT32_variables
      cmp   [problem_partition],0
      jne   search_partitions_ide2
      inc   byte [0x40003]
      call  partition_data_transfer
      add   [transfer_adress],100
      inc   [fat32part]
      jmp   search_partitions_ide1_1

 search_partitions_ide2:
      test  [0x40001],byte 0x4
      jz   search_partitions_ide3
        mov   [hdbase],0x170
        mov   [hdid],0x0
        mov   [hdpos],3
      mov  [fat32part],1
 search_partitions_ide2_1:
      call  set_FAT32_variables
      cmp   [problem_partition],0
      jne   search_partitions_ide3
      inc   byte [0x40004]
      call  partition_data_transfer
      add   [transfer_adress],100
      inc   [fat32part]
      jmp   search_partitions_ide2_1

 search_partitions_ide3:
      test  [0x40001],byte 0x1
      jz   end_search_partitions_ide
        mov   [hdbase],0x170
        mov   [hdid],0x10
        mov   [hdpos],4
      mov  [fat32part],1
 search_partitions_ide3_1:
      call  set_FAT32_variables
      cmp   [problem_partition],0
      jne   end_search_partitions_ide
      inc   byte [0x40005]
      call  partition_data_transfer
      add   [transfer_adress],100
      inc   [fat32part]
      jmp   search_partitions_ide3_1

 
partition_data_transfer: 
     mov edi,[transfer_adress]
     mov esi,PARTITION_START
     xor ecx,ecx
     mov cx,69   ;100
     rep movsb
     ret
transfer_adress dd 0
partition_data_transfer_1:
     cli 
     mov edi,PARTITION_START
     mov esi,[transfer_adress]
     xor ecx,ecx
     mov cx,69  ;100
     rep movsb
     sti
     ret

 end_search_partitions_ide:

;PARTITION_START      dd 0x3f
;PARTITION_END        dd 0
;SECTORS_PER_FAT      dd 0x1f3a
;NUMBER_OF_FATS       dd 0x2
;SECTORS_PER_CLUSTER  dd 0x8
;BYTES_PER_SECTOR     dd 0x200   ; Note: if BPS <> 512 need lots of changes
;ROOT_CLUSTER         dd 2       ; first rootdir cluster
;FAT_START            dd 0       ; start of fat table
;ROOT_START           dd 0       ; start of rootdir (only fat16)
;ROOT_SECTORS         dd 0       ; count of rootdir sectors (only fat16)
;DATA_START           dd 0       ; start of data area (=first cluster 2)
;LAST_CLUSTER         dd 0       ; last availabe cluster
;ADR_FSINFO           dd 0       ; used only by fat32
;
;fatRESERVED          dd 0x0FFFFFF6
;fatBAD               dd 0x0FFFFFF7
;fatEND               dd 0x0FFFFFF8
;fatMASK              dd 0x0FFFFFFF
;
;fat_type             db 0       ; 0=none, 16=fat16, 32=fat32

