#include "rskos.h"

#ifndef RS_KOS

#include "rs/rstexture.h"
#include "rs/rsgl.h"
#include "rs/rsshader.h"

#include "rs/rsaudio.h"

#include <stdlib.h>
#include <string.h>

//unsigned char* rskos_malloc(unsigned int bytes_count) {
//    return malloc(bytes_count);
//};
//
//void rskos_free(unsigned char* pointer) {
//    free(pointer);
//};
//
//void rskos_memset(unsigned char* pointer, unsigned char value, unsigned int bytes_count) {
//    memset(pointer, value, bytes_count);
//};
//
//void rskos_memcpy(unsigned char* dest, unsigned char* src, unsigned int bytes_count) {
//    memcpy(dest, src, bytes_count);
//};



unsigned int rskos_get_time() {
    
    return rs_app.app_time;
    
};

void rskos_draw_area(int x, int y, int w, int h, int k_scale, unsigned char *data, unsigned char *scaled_buffer, int image_format) {
    
    int bpp = image_format == RSKOS_BGR ? 3 : 4;

//    int i, j;
//    
//    for (i = 0; i < h*k_scale; i++) {
//        for (j = 0; j < w*k_scale; j++) {
//            scaled_buffer[ (i*w*k_scale + j)*3 + 0] = data[ ( (i/k_scale)*w + (j/k_scale) )*4 + 0];
//            scaled_buffer[ (i*w*k_scale + j)*3 + 1] = data[ ( (i/k_scale)*w + (j/k_scale) )*4 + 1];
//            scaled_buffer[ (i*w*k_scale + j)*3 + 2] = data[ ( (i/k_scale)*w + (j/k_scale) )*4 + 2];
//        };
//    };


    




	glViewport(0, 0, rs_app.width, rs_app.height); 
    glClearColor( 0.2596078431, 0.2815686275, 0.3929411765, 1.0 ); // #98d0ed
    //glClearColor( 0.0, 0.4, 0.1 + 0.5*0.001*(rs_get_time()%1024) , 1.0 ); 

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // <-- FBO
	
	
	
	
	
    
    //rs_tx_t tex = rs_tx_create_from_data(w*k_scale, h*k_scale, 3, 0, 1, scaled_buffer);
    rs_tx_t tex = rs_tx_create_from_data(w, h, bpp, 0, 1, data);
    
    glBindTexture(GL_TEXTURE_2D, tex);
    
    rs_sh_use(DefaultShader);
    
    rs_mx_identity_modelview();
    rs_mx_ortho_proj();
    
    glUniformMatrix4fv( DefaultShader[RS_SH_PROJ_ID], 1, GL_FALSE, rs_reg.mx_proj );
    glUniformMatrix4fv( DefaultShader[RS_SH_MODELVIEW_ID], 1, GL_FALSE, rs_reg.mx_modelview );
    
    glrsDrawRect(x, y, x+w*k_scale, y+h*k_scale, DefaultShader[RS_SH_POS_ID], DefaultShader[RS_SH_UV_ID]); 
    
    rs_tx_destroy(&tex);
    
    
    

    rsDoSwapBuffers();
    
};


void rskos_resize_window(int w, int h) {
    //
};

void rskos_get_screen_size(unsigned int  *pw, unsigned int  *ph) {
    *pw = rs_app.width + 50;
    *ph = rs_app.height + 50;
};

void rskos_exit() {
    rsAppExit();
};




int rskos_file_save(char *filename, unsigned char *data, int length) {
    FILE *fp;
    
    fp = fopen(filename, "w");
    if (!fp) {
        return 0;
    };
    
    fwrite(data, 1, length, fp);
    fclose(fp);
    
    return 1;
    
};

int rskos_file_load(char *filename, unsigned char *data, int length) {
    FILE *fp;
    
    fp = fopen(filename, "r");
    if (!fp) {
        return 0;
    };
    
    fread(data, 1, length, fp);
    fclose(fp);
    
    return 1;
};









//void rskos_snd_init() {
//    //
//    
//    rs_audio
//    
//};

void rskos_snd_create_buffer(SNDBUF *hbuf, signed short *buffer, unsigned int length) {
    //
    
    rs_sound_t *snd = malloc( sizeof(rs_sound_t) );   
    rs_sound_create_from_data(snd, length*2, (unsigned char*) buffer);
    
    *hbuf = snd;
    
};

void rskos_snd_update_buffer(SNDBUF *hbuf, signed short *buffer, unsigned int length) {
    
    rs_sound_destroy(*hbuf);
    rskos_snd_create_buffer(hbuf, buffer, length);
    
};

void rskos_snd_play(SNDBUF *hbuf, unsigned int mode) {
    
    rs_sound_play(*hbuf);
    
};

void rskos_snd_stop(SNDBUF *hbuf) {
    rs_sound_stop(*hbuf);
};


#else

    #include "rs/rsplatform.h"
    
    
    #pragma pack(push,1)
     
    typedef struct rskos_file_struct_t {
        
        unsigned int func_num;
        unsigned int offset;
        unsigned int flags;
        unsigned int length;
        unsigned char *data;
        unsigned char  zero;
        char *filename;
        
    } rskos_file_struct_t;
     
    #pragma pack(pop)    
    


    unsigned int rskos_get_time() {
        return 1;
    };
    
    
    

    
    
    
    
     
    int rskos_file_load(char *filename, unsigned char *data, int length) {
 
//        char filename_abs[] = "/sys/games/************************";
//        memcpy( filename_abs[ strchr(filename_abs, "*") ], filename, strlen(filename)+1 );
        
        rskos_file_struct_t file_struct;
        file_struct.func_num = 0;
        file_struct.offset = 0;
        file_struct.flags = 0;
        file_struct.length = length;
        file_struct.data = data;
        file_struct.zero = 0;
        file_struct.filename = filename;
     
        asm volatile ("int $0x40"::"a"(70), "b"(&file_struct) : "memory");
        
        return 1;
    }
    
    
    
    int rskos_file_save(char *filename, unsigned char *data, int length) {

//        char filename_abs[] = "/sys/games/************************";
//        memcpy( filename_abs[ strchr(filename_abs, "*") ], filename, strlen(filename)+1 );
        
        rskos_file_struct_t file_struct;
        file_struct.func_num = 2;
        file_struct.offset = 0;
        file_struct.flags = 0;
        file_struct.length = length;
        file_struct.data = data;
        file_struct.zero = 0;
        file_struct.filename = filename;
     
        asm volatile ("int $0x40"::"a"(70), "b"(&file_struct) : "memory" );
        
        return 1;
    }
    

    
    

    
    
    
    

    void rskos_draw_area(int x, int y, int w, int h, int k_scale, unsigned char *data, unsigned char *scaled_buffer, int image_format) {

        int i, j;
        
        for (i = 0; i < h*k_scale; i++) {
            for (j = 0; j < w*k_scale; j++) {
                scaled_buffer[ (i*w*k_scale + j)*3 + 0] = data[ ( (i/k_scale)*w + (j/k_scale) )*4 + 0];
                scaled_buffer[ (i*w*k_scale + j)*3 + 1] = data[ ( (i/k_scale)*w + (j/k_scale) )*4 + 1];
                scaled_buffer[ (i*w*k_scale + j)*3 + 2] = data[ ( (i/k_scale)*w + (j/k_scale) )*4 + 2];
            };
        };

        kol_paint_image(0, 0, w*k_scale, h*k_scale, scaled_buffer);

    };
    
    void rskos_resize_window(int w, int h) {
        
        // !!!!!!!!! add define-fix here
        
        w = -1 + w + 10; // 2 x 5px border
        h = -1 + kol_skin_height() + h + 5; // bottom 5px border
        
        asm volatile ("int $0x40"::"a"(67), "b"(-1), "c"(-1), "d"(w), "S"(h));
        
    };
    
    void rskos_get_screen_size(unsigned int  *pw, unsigned int  *ph) {
        kol_screen_get_size(pw, ph);
    };
    
    void rskos_exit() {
        kol_exit();
    };

    #define fmt PCM_1_16_16 // 1 channel, 16 bit per sample, 16 kHz

    void rskos_snd_create_buffer(SNDBUF *phbuf, signed short *buffer, unsigned int length_samples) {
        unsigned int snd_format = fmt;
        CreateBuffer(snd_format|PCM_STATIC, length_samples*2, phbuf);
        int offset = 0; 
        SetBuffer(*phbuf, (void*)buffer, offset, length_samples*2);
    };
    
    void rskos_snd_update_buffer(SNDBUF *phbuf, signed short *buffer, unsigned int length_samples) {
        unsigned int snd_format = fmt;
//        CreateBuffer(snd_format|PCM_STATIC, length, phbuf);
        int offset = 0; 
        SetBuffer(*phbuf, (void*)buffer, offset, length_samples*2);
    };

    void rskos_snd_play(SNDBUF *phbuf, unsigned int mode) {
        SetBufferPos(*phbuf, 0);
        PlayBuffer(*phbuf, 0);
    };

    void rskos_snd_stop(SNDBUF *phbuf) {
        StopBuffer(*phbuf);
    };



#endif
