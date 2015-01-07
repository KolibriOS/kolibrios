#ifndef _I965_FOURCC_H_
#define _I965_FOURCC_H_

#ifndef VA_FOURCC_YV16
#define VA_FOURCC_YV16 VA_FOURCC('Y','V','1','6')
#endif

#ifndef VA_FOURCC_I420
#define VA_FOURCC_I420 VA_FOURCC('I','4','2','0')
#endif

/*
 * VA_FOURCC_IA44 is an exception because the va.h already
 * defines the AI44 as VA_FOURCC('I', 'A', '4', '4').
 */
#ifndef VA_FOURCC_IA44
#define VA_FOURCC_IA44 VA_FOURCC('A','I','4','4')
#endif

#ifndef VA_FOURCC_IA88
#define VA_FOURCC_IA88 VA_FOURCC('I','A','8','8')
#endif

#ifndef VA_FOURCC_AI88
#define VA_FOURCC_AI88 VA_FOURCC('A','I','8','8')
#endif

#ifndef VA_FOURCC_IMC1
#define VA_FOURCC_IMC1 VA_FOURCC('I','M','C','1')
#endif

#ifndef VA_FOURCC_YVY2
#define VA_FOURCC_YVY2 VA_FOURCC('Y','V','Y','2')
#endif

#define I965_MAX_PLANES         4
#define I965_MAX_COMONENTS      4

#define I965_COLOR_YUV          0
#define I965_COLOR_RGB          1
#define I965_COLOR_INDEX        2

typedef struct {
    uint8_t plane;                      /* the plane which the pixel belongs to */
    uint8_t offset;                     /* bits offset within a pixel in the plane */
} i965_component_info;

typedef struct {
    uint32_t fourcc;                    /* fourcc */
    uint32_t format;                    /* 0: YUV, 1: RGB, 2: Indexed format */
    uint32_t subsampling;               /* Sub sampling */
    uint8_t flag;                       /* 1: only supported by vaCreateSurfaces(), 2: only supported by vaCreateImage(), 3: both */
    uint8_t hfactor;                    /* horizontal sampling factor */
    uint8_t vfactor;                    /* vertical sampling factor */
    uint8_t num_planes;                 /* number of planes */
    uint8_t bpp[I965_MAX_PLANES];       /* bits per pixel within a plane */
    uint8_t num_components;             /* number of components */
    /*
     * Components in the array are ordered in Y, U, V, A (up to 4 components)
     * for YUV formats, R, G, B, A (up to 4 components) for RGB formats and
     * I, A (2 components) for indexed formats
     */
    i965_component_info components[I965_MAX_COMONENTS];
} i965_fourcc_info;

extern const i965_fourcc_info *get_fourcc_info(unsigned int);

#endif /* _I965_FOURCC_H_ */
