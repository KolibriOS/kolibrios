//Asper

//library
dword libimg = #alibimg;
char alibimg[21] = "/sys/lib/libimg.obj\0"; //"libimg.obj\0";

dword libimg_init = #alibimg_init;
dword img_is_img  = #aimg_is_img;
dword img_to_rgb2 = #aimg_to_rgb2;
dword img_decode  = #aimg_decode;
dword img_destroy = #aimg_destroy;
dword img_draw    = #aimg_draw;
//dword img_flip    = #aimg_flip;
//dword img_rotate  = #aimg_rotate;

dword  am1__ = 0x0;
dword  bm1__ = 0x0;

//import  libimg                     , \
char alibimg_init[9] = "lib_init\0";
char aimg_is_img[11]  = "img_is_img\0";
char aimg_to_rgb2[12] = "img_to_rgb2\0";
char aimg_decode[11]  = "img_decode\0";
char aimg_destroy[12] = "img_destroy\0";
char aimg_draw[9]    = "img_draw\0";
//char aimg_flip[9]    = "img_flip\0";
//char aimg_rotate[11]  = "img_rotate\0 ";



dword load_image(dword filename)
{
        //align 4
        dword img_data=0;
        dword img_data_len=0;
        dword fh=0;
        dword image=0;

        byte tmp_buf[40];
        $and     img_data, 0
        //$mov     eax, filename
        //$push    eax        
        //invoke  file.open, eax, O_READ
        file_open stdcall (filename, O_READ);
        $or      eax, eax
        $jnz      loc05  
        $stc
        return 0;
    @loc05:    
        $mov     fh, eax
        //invoke  file.size
        file_size stdcall (filename);
        $mov     img_data_len, ebx
        //stdcall mem.Alloc, ebx
        mem_Alloc(EBX);
        
        $test    eax, eax
        $jz      error_close
        $mov     img_data, eax
        //invoke  file.read, [fh], eax, [img_data_len]
        file_read stdcall (fh, EAX, img_data_len);
        $cmp     eax, -1
        $jz      error_close
        $cmp     eax, img_data_len
        $jnz     error_close
        //invoke  file.close, [fh]
        file_close stdcall (fh);
        $inc     eax
        $jz      error_
//; img.decode checks for img.is_img
//;       //invoke  img.is_img, [img_data], [img_data_len]
//;       $or      eax, eax
//;       $jz      exit
        //invoke  img.decode, [img_data], [img_data_len], 0
        EAX=img_data;
        img_decode stdcall (EAX, img_data_len,0);
        $or      eax, eax
        $jz      error_
        $cmp     image, 0
        $pushf
        $mov     image, eax
        //call    init_frame
        $popf
        //call    update_image_sizes
        mem_Free(img_data);//free_img_data(img_data);
        $clc
        return image;

@error_free:
        //invoke  img.destroy, [image]
        img_destroy stdcall (image);
        $jmp     error_

@error_pop:
        $pop     eax
        $jmp     error_
@error_close:
        //invoke  file.close, [fh]
        file_close stdcall (fh);
@error_:
        mem_Free(img_data);
        $stc
        return 0;
}
