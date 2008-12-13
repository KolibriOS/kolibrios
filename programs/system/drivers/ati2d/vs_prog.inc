
typedef unsigned int u32_t;


typedef enum
{
    VS_OUT_POS   = 0,
    VS_OUT_PSIZE,
    VS_OUT_COL0,
    VS_OUT_COL1,
    VS_OUT_COL2,
    VS_OUT_COL3,
    VS_OUT_TEX0,
    VS_OUT_TEX1,
    VS_OUT_TEX2,
    VS_OUT_TEX3,
    VS_OUT_TEX4,
    VS_OUT_TEX5,
    VS_OUT_TEX6,
    VS_OUT_TEX7,
    VS_OUT_FOG,
    VS_OUT_MAX = 0xFFFFFFFF
}v_out_t;


#if 0
    vs_1_1

    dcl_position   v0
    dcl_color      v1
    dcl_color1     v2
    dcl_fog        v3
    dcl_psize      v4
    dcl_texcoord   v5
    dcl_texcoord1  v6

    mov oPos,    v0

    mov oD0,     v1
    mov oD1,     v2

    mov oFog,    v3.x
    mov oPts,    v4.x
    mov oT0,     v5
    mov oT1,     v6

#endif

const u32_t vs11[] =
{
    0xfffe0101, 0x0000001f, 0x80000000, 0x900f0000, 0x0000001f, 0x8000000a,
    0x900f0001, 0x0000001f, 0x8001000a, 0x900f0002, 0x0000001f, 0x8000000b,
    0x900f0003, 0x0000001f, 0x80000004, 0x900f0004, 0x0000001f, 0x80000005,
    0x900f0005, 0x0000001f, 0x80010005, 0x900f0006, 0x00000001, 0xc00f0000,
    0x90e40000, 0x00000001, 0xd00f0000, 0x90e40001, 0x00000001, 0xd00f0001,
    0x90e40002, 0x00000001, 0xc00f0001, 0x90000003, 0x00000001, 0xc00f0002,
    0x90000004, 0x00000001, 0xe00f0000, 0x90e40005, 0x00000001, 0xe00f0001,
    0x90e40006, 0x0000ffff
};

char *sz_vs_command[] =
{
    "nop",
    "mov",
    "add",
    "sub",
    "mad",
    "mul",
    "rcp",
    "rsq",
    "dp3",
    "dp4",
    "min",
    "max",
    "slt",
    "sge",
    "exp",
    "log",
    "lit",
    "dst",
    "lrp",
    "frc",
    "m4x4",
    "m4x3",
    "m3x4",
    "m3x3",
    "m3x2",
};

/*
char *sz_ps_command[] =
{
    texcoord
    texkill
    tex
    texbem
    texbeml
    texreg2ar
    texreg2gb
    texm3x2pad
    texm3x3tex
    texm3x3pad
    texm3x3tex
    texm3x3diff
    texm3x3spec
    texm3x3vspec
    expp
    logp
    cnd
    def
    texreg2rgb
    texdp3tex
    texm3x2depth
    texdp3
    texm3x3
    texdepth
    cmp
    bem
}
*/

char *szusage[]=
{
   "position",
   "blendweight",
   "blendindices",
   "normal",
   "psize",
   "texcoord",
   "tangent",
   "binormal",
   "tessfactor",
   "positiont",
   "color",
   "fog",
   "depth",
   "sample"
};
char *sztype[]=
{
    "r",
    "v",
    "c"
    "a",
    "t",
    "rasout",
    "attrout",
    "texcrdout",
    "output",
    "constint",
    "colorout",
    "depthout",
    "sampler",
    "const2",
    "const3",
    "const4",
    "constbool",
    "loop",
    "tempfloat16",
    "misctype",
    "label",
    "predicate"
};


typedef struct
{
  u32_t minor: 8;
  u32_t major: 8;
  u32_t type :16;
}version_t;

typedef struct
{
  u32_t type:5;
  u32_t rsv :11;
  u32_t ind :4;
  u32_t rsv2:11;
  u32_t sign:1;
}usage_t;

typedef struct
{
  u32_t ind   :11;
  u32_t typeh :2;
  u32_t rsv   :3;
  u32_t wr    :4;
  u32_t mod   :4;
  u32_t scale :4;
  u32_t typel :3;
  u32_t sign  :1;
}dst_t;

typedef struct
{
  u32_t ind   :11;
  u32_t rsv   :5;
  u32_t swzl  :8;
  u32_t mod   :4;
  u32_t typel :3;
  u32_t sign  :1;
}src_t;


int parse_vs(const u32_t *stream);

static void assign_outputs();

int translate_vs(const u32_t *stream);


u32_t  vs_out_written;
u32_t  inp_mask;

u32_t  vs_outputs[16];

int main()
{
    version_t *ver;

    ver = (version_t*)vs11;

    if(ver->type == 0xFFFE)
    {
        printf("vs_%d_%d\n\n",ver->major,ver->minor);
        if( parse_vs(vs11+1) )
            translate_vs(vs11+1);
    };

    return 0;
};

static char txt_swzl[4] = {'x','y','z','w'};
static char *txt_mod[2] = { "","_sat"};

int parse_vs(const u32_t *stream)
{
  dst_t  *dst;
  src_t  *src;

  u32_t  swzl;
  u32_t  wr;

  char   szswzl[5];
  char   szwm[5];

  int    i,j;

  while(1)
  {
    op_type_t instr = *stream++ & 0xFFFF;

    switch( instr )
    {
      case D3DSIO_MOV:
        dst = (dst_t*)stream++;
        src = (src_t*)stream++;

        swzl = src->swzl;
        wr = dst->wr;

        for(i=0,j=0; i < 4; i++)
        {
           szswzl[i] = txt_swzl[swzl&3];
           swzl>>=2;
           if(wr & (1<<i))
           szwm[j++] = txt_swzl[i];
        };
        szswzl[4] = 0;
        szwm[j] = 0;

        switch(dst->typel)
        {
           case 4:                   // Rasterizer Register File
              if(dst->ind == 0)
                 vs_out_written |= (1 << VS_OUT_POS);
              else if (dst->ind == 1)
                 vs_out_written |= (1 << VS_OUT_FOG);
              else if (dst->ind == 2)
                 vs_out_written |= (1 << VS_OUT_PSIZE);
              else
                 printf("invalid raster register %d",dst->ind);
              break;

           case 5:     // Attribute Output Register File
              if(dst->ind == 0)
                 vs_out_written |= (1 << VS_OUT_COL0);
              else if (dst->ind == 1)
                 vs_out_written |= (1 << VS_OUT_COL1);
              else
                 printf("invalid attribute register %d",dst->ind);
              break;

           case 6:          // Texture Coordinate Output Register File
              if(dst->ind < 8)
                 vs_out_written |= (1 << (VS_OUT_TEX0+dst->ind));
              else
                 printf("invalid texture register %d",dst->ind);
        };
        printf("%s%s %s%d.%s,\t %s%d.%s\n",sz_vs_command[instr],txt_mod[dst->mod],
               sztype[dst->typel],dst->ind,szwm,
               sztype[src->typel],src->ind,szswzl);
        break;

      case D3DSIO_DCL:
        parse_dcl(stream);
        stream+=2;
        break;
      case 0xFFFF:
        return 1;

      default:
        return 0;
    };
  };
};


int parse_dcl(const u32_t *stream)
{
  usage_t *usage;
  dst_t   *dst;
  int dsttype;
  char szwm[5];
  int i;
  u32_t wr;

  usage = (usage_t*)stream++;
  dst = (dst_t*)stream++;
  dsttype = (dst->typeh << 4) | dst->typel;
  wr = dst->wr;

  for(i=0; wr; i++, wr>>=1)
  {
    if(wr & 1)
      szwm[i] = txt_swzl[i];
  };
  szwm[i] = 0;

  printf("dcl_%s%d \t\t %s%d.%s\n",szusage[usage->type],usage->ind,
         sztype[dsttype],dst->ind, szwm);

  return 2;
}



int translate_dcl(const u32_t *stream);
int translate_mov(const u32_t *stream);

int translate_vs(const u32_t *stream)
{
  assign_outputs();

  while(1)
  {
    op_type_t instr = *stream++ & 0xFFFF;

    switch( instr )
    {
      case D3DSIO_MOV:
        translate_mov(stream);
        stream+=2;
        break;
      case D3DSIO_DCL:
        translate_dcl(stream);
        stream+=2;
        break;
      case 0xFFFF:
        return 1;

      default:
        return 0;
    };
  };
};


int translate_dcl(const u32_t *stream)
{



  return 1;
};

u32_t inst[4];

#include "r300_vertprog.h"

/**
 * Swizzle indexes.
 * Do not change!
 */
/*@{*/
#define SWIZZLE_X    0
#define SWIZZLE_Y    1
#define SWIZZLE_Z    2
#define SWIZZLE_W    3
#define SWIZZLE_ZERO 4   /**< For SWZ instruction only */
#define SWIZZLE_ONE  5   /**< For SWZ instruction only */
#define SWIZZLE_NIL  7   /**< used during shader code gen (undefined value) */
/*@}*/

#define __CONST(x, y)	\
	(PVS_SRC_OPERAND(t_src_index(vp, &src[x]),	\
			   t_swizzle(y),	\
			   t_swizzle(y),	\
			   t_swizzle(y),	\
			   t_swizzle(y),	\
			   t_src_class(src[x].File), \
			   VSF_FLAG_NONE) | (src[x].RelAddr << 4))

static unsigned long t_swizzle(u32_t swizzle)
{
/* this is in fact a NOP as the Mesa SWIZZLE_* are all identical to VSF_IN_COMPONENT_* */
	return swizzle;
}

static unsigned long t_dst_mask(u32_t mask)
{
	/* WRITEMASK_* is equivalent to VSF_FLAG_* */
	return mask & VSF_FLAG_ALL;
}

static unsigned long t_dst_class(int file)
{

	switch (file) {
	case 0:                             //D3DSPR_TEMP
		return PVS_DST_REG_TEMPORARY;
	case 3:                             //D3DSPR_ADDR
		return PVS_DST_REG_A0;
	case 4:                             //D3DSPR_RASTOUT
	case 5:
	case 6:                             //D3DSPR_TEXCRDOUT
		return PVS_DST_REG_OUT;

		/*
		   case PROGRAM_INPUT:
		   case PROGRAM_LOCAL_PARAM:
		   case PROGRAM_ENV_PARAM:
		   case PROGRAM_NAMED_PARAM:
		   case PROGRAM_STATE_VAR:
		   case PROGRAM_WRITE_ONLY:
		   case PROGRAM_ADDRESS:
		 */
	default:
		printf("problem in %s", __FUNCTION__);
		return -1;
	}
}

static unsigned long t_dst_index(dst_t *dst)
{
  switch(dst->typel)
  {
    case 4:
        if(dst->ind == 0)
           return vs_outputs[VS_OUT_POS];
        else if (dst->ind == 1)
           return vs_outputs[VS_OUT_FOG];
        else if (dst->ind == 2)
           return vs_outputs[VS_OUT_PSIZE];
        break;
    case 5:
        if(dst->ind == 0)
           return vs_outputs[VS_OUT_COL0];
        else if (dst->ind == 1)
           return vs_outputs[VS_OUT_COL1];

    case 6:
        return vs_outputs[VS_OUT_TEX0+dst->ind];

    default:
      return dst->ind;
  }
}


static void assign_outputs()
{
	int i;
	int cur_reg = 0;

	for (i = 0; i < 16; i++)
		vs_outputs[i] = -1;

//	assert(vs_out_written & (1 << VS_OUT_POS));

	if (vs_out_written & (1 << VS_OUT_POS)) {
		 vs_outputs[VS_OUT_POS] = cur_reg++;
	}

	if (vs_out_written & (1 << VS_OUT_PSIZE)) {
		 vs_outputs[VS_OUT_PSIZE] = cur_reg++;
	}

	if (vs_out_written & (1 << VS_OUT_COL0)) {
		 vs_outputs[VS_OUT_COL0] = cur_reg++;
	}

	if (vs_out_written & (1 << VS_OUT_COL1)) {
		 vs_outputs[VS_OUT_COL1] = vs_outputs[VS_OUT_COL0] + 1;           // ???
		 cur_reg = vs_outputs[VS_OUT_COL1] + 1;
	}

#if 0
	if (vp->key.OutputsWritten & (1 << VERT_RESULT_FOGC)) {  //fog must be in
		vp->outputs[VERT_RESULT_FOGC] = cur_reg++;             //one of the color regs
	}
#endif

	for (i = VS_OUT_TEX0; i <= VS_OUT_TEX7; i++) {
		if (vs_out_written & (1 << i)) {
			  vs_outputs[i] = cur_reg++;
		}
	}
}


int translate_mov(const u32_t *stream)
{

   dst_t *dst = (dst_t*)stream++;
   src_t *src = (src_t*)stream++;

   int swzl = src->swzl;
   int wr = dst->wr;


	 inst[0] = PVS_OP_DST_OPERAND(VE_ADD,
				     0,
				     0,
				     t_dst_index(dst),
				     (dst->wr),
				     t_dst_class(dst->typel));

	//inst[1] = t_src(vp, &src[0]);
//	inst[2] = __CONST(0, SWIZZLE_ZERO);
	//inst[3] = __CONST(0, SWIZZLE_ZERO);
    printf("inst_0  %x\n", inst[0]);
	  return 1;
}

/*
static GLuint *r300TranslateOpcodeMOV(struct r300_vertex_program *vp,
				      struct prog_instruction *vpi,
				      GLuint * inst,
				      struct prog_src_register src[3])
{
	//ADD RESULT 1.X Y Z W PARAM 0{} {X Y Z W} PARAM 0{} {ZERO ZERO ZERO ZERO}

	inst[0] = PVS_OP_DST_OPERAND(VE_ADD,
				     GL_FALSE,
				     GL_FALSE,
				     t_dst_index(vp, &vpi->DstReg),
				     t_dst_mask(vpi->DstReg.WriteMask),
				     t_dst_class(vpi->DstReg.File));
	inst[1] = t_src(vp, &src[0]);
	inst[2] = __CONST(0, SWIZZLE_ZERO);
	inst[3] = __CONST(0, SWIZZLE_ZERO);

	return inst;
}
 */
