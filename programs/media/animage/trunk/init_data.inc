;---------------------------------------------------------
;--------initializate data and params of program----------
;---------------------------------------------------------
   mov [Scroll1CoordinatX],8+1
   mov [Scroll2CoordinatY],20+15+1+50+3
   mov [Window_SizeX],640
   mov [Window_SizeY],220
   mov [Window_CordinatX],50
   mov [Window_CordinatY],50
   mov [WorkScreen_SizeX],100
   mov [WorkScreen_SizeY],100
   mov [MaxWorkScreen_SizeX],100
   mov [MaxWorkScreen_SizeY],100
   mov [ScreenPointer],0x19000
   and [PosX],0
   and [PosY],0
   mov [Picture_SizeX],640;400
   mov [Picture_SizeY],400;280
   mov [k],1
   mov [PointerToPicture],0x19000+(1200*1000*3)+50*(20*20*3)+500000
   mov [PointerToCopyPicture],0x19000+(1200*1000*3)+50*(20*20*3)+500000+(640*400*3)
   mov [PointerToCopyPicture2],0x19000+(1200*1000*3)+50*(20*20*3)+500000+(640*400*3)*2
   mov [PointerToEditBufer],0x19000+(1200*1000*3)+50*(20*20*3)+500000+(640*400*3)*3
   mov [PointerToSpriteBufer],0x19000+(1200*1000*3)+50*(20*20*3)+500000+(640*400*3)*4
   mov [PointerToPalette],0x19000+(1200*100*3)+50*(20*20*3)+1
   mov [ReserveArray],0x19000+(1200*1000)*3+50*(20*20*3)+8
   and [save_flag],0
   mov [line_width],1
   mov [lastik_is_active],0
   and [crossing],0
   and [finishing_crossing],0
   and [number_undo],0
   and [instrument_used],0
   and [DrawSprite_flag],0
   and [extended_memory],0

   ;mov eax,68
   ;mov ebx,11
   ;int 0x40

   ;mov eax,68
   ;mov ebx,12
   ;mov ecx,60000
   ;int 0x40

   ;mov [PointerToIcons],eax

   finit
