{*******************************************************************

          KolibriOS TinyGL unit

          Copyright (c) 2025 Delphi SDK for KolibriOS team

********************************************************************}

unit TinyGL;

interface

uses
  KolibriOS;

type
  GLEnum     = LongInt;
  GLBoolean  = Boolean;
  GLBitfield = LongInt;
  GLByte     = Shortint;
  GLShort    = SmallInt;
  GLInt      = LongInt;
  GLSizei    = LongInt;
  GLUByte    = Byte;
  GLUShort   = Word;
  GLUInt     = LongInt;
  GLFloat    = Single;
  GLClampf   = Single;
  GLDouble   = Double;
  GLClampd   = Double;
  GLVoid     = Pointer;

  PGLBoolean = ^GLBoolean;
  PGLByte    = ^GLByte;
  PGLShort   = ^GLShort;
  PGLInt     = ^GLInt;
  PGLSizei   = ^GLSizei;
  PGLUByte   = ^GLUByte;
  PGLUShort  = ^GLUShort;
  PGLUInt    = ^GLUInt;
  PGLClampf  = ^GLClampf;
  PGLFloat   = ^GLFloat;
  PGLDouble  = ^GLDouble;
  PGLClampd  = ^GLClampd;
  PGLVoid    = ^GLVoid;

  TGLArrayi4 = array [0..3] of GLInt;
  TGLArrayf3 = array [0..2] of GLFloat;
  TGLArrayf4 = array [0..3] of GLFloat;

  _GLUquadricObj = record end;
  GLUquadricObj = ^_GLUquadricObj;

  TKOSGLContext = packed record
    GLContext: Pointer;
    XSize, YSize: LongInt;
    DX, DY, X, Y: LongInt;
  end;

const
// Error Code
  GL_NO_ERROR          = 0;
  GL_INVALID_ENUM      = $0500;
  GL_INVALID_VALUE     = $0501;
  GL_INVALID_OPERATION = $0502;
  GL_STACK_OVERFLOW    = $0503;
  GL_STACK_UNDERFLOW   = $0504;
  GL_OUT_OF_MEMORY     = $0505;

// Attribute Mask
  GL_CURRENT_BIT         = $00000001;
  GL_POINT_BIT           = $00000002;
  GL_LINE_BIT            = $00000004;
  GL_POLYGON_BIT         = $00000008;
  GL_POLYGON_STIPPLE_BIT = $00000010;
  GL_PIXEL_MODE_BIT      = $00000020;
  GL_LIGHTING_BIT        = $00000040;
  GL_FOG_BIT             = $00000080;
  GL_DEPTH_BUFFER_BIT    = $00000100;
  GL_ACCUM_BUFFER_BIT    = $00000200;
  GL_STENCIL_BUFFER_BIT  = $00000400;
  GL_VIEWPORT_BIT        = $00000800;
  GL_TRANSFORM_BIT       = $00001000;
  GL_ENABLE_BIT          = $00002000;
  GL_COLOR_BUFFER_BIT    = $00004000;
  GL_HINT_BIT            = $00008000;
  GL_EVAL_BIT            = $00010000;
  GL_LIST_BIT            = $00020000;
  GL_TEXTURE_BIT         = $00040000;
  GL_SCISSOR_BIT         = $00080000;
  GL_ALL_ATTRIB_BITS     = $000fffff;

// Boolean
  GL_FALSE = False;
  GL_TRUE  = not GL_FALSE;

// Begin Mode
  GL_POINTS         = $0000;
  GL_LINES          = $0001;
  GL_LINE_LOOP      = $0002;
  GL_LINE_STRIP     = $0003;
  GL_TRIANGLES      = $0004;
  GL_TRIANGLE_STRIP = $0005;
  GL_TRIANGLE_FAN   = $0006;
  GL_QUADS          = $0007;
  GL_QUAD_STRIP     = $0008;
  GL_POLYGON        = $0009;

// Accum Op
  GL_ACCUM  = $0100;
  GL_LOAD   = $0101;
  GL_RETURN = $0102;
  GL_MULT   = $0103;
  GL_ADD    = $0104;

// Alpha Function
  GL_NEVER    = $0200;
  GL_LESS     = $0201;
  GL_EQUAL    = $0202;
  GL_LEQUAL   = $0203;
  GL_GREATER  = $0204;
  GL_NOTEQUAL = $0205;
  GL_GEQUAL   = $0206;
  GL_ALWAYS   = $0207;

// Blending Factor Dst
  GL_ZERO                = 0;
  GL_ONE                 = 1;
  GL_SRC_COLOR           = $0300;
  GL_ONE_MINUS_SRC_COLOR = $0301;
  GL_SRC_ALPHA           = $0302;
  GL_ONE_MINUS_SRC_ALPHA = $0303;
  GL_DST_ALPHA           = $0304;
  GL_ONE_MINUS_DST_ALPHA = $0305;

// Blending Factor Src
  GL_DST_COLOR           = $0306;
  GL_ONE_MINUS_DST_COLOR = $0307;
  GL_SRC_ALPHA_SATURATE  = $0308;

// Draw Buffer Mode
  GL_NONE           = 0;
  GL_FRONT_LEFT     = $0400;
  GL_FRONT_RIGHT    = $0401;
  GL_BACK_LEFT      = $0402;
  GL_BACK_RIGHT     = $0403;
  GL_FRONT          = $0404;
  GL_BACK           = $0405;
  GL_LEFT           = $0406;
  GL_RIGHT          = $0407;
  GL_FRONT_AND_BACK = $0408;
  GL_AUX0           = $0409;
  GL_AUX1           = $040A;
  GL_AUX2           = $040B;
  GL_AUX3           = $040C;

// Feed Back Mode
  GL_2D               = $0600;
  GL_3D               = $0601;
  GL_3D_COLOR         = $0602;
  GL_3D_COLOR_TEXTURE = $0603;
  GL_4D_COLOR_TEXTURE = $0604;

// Feed Back Token
  GL_PASS_THROUGH_TOKEN = $0700;
  GL_POINT_TOKEN        = $0701;
  GL_LINE_TOKEN         = $0702;
  GL_POLYGON_TOKEN      = $0703;
  GL_BITMAP_TOKEN       = $0704;
  GL_DRAW_PIXEL_TOKEN   = $0705;
  GL_COPY_PIXEL_TOKEN   = $0706;
  GL_LINE_RESET_TOKEN   = $0707;

// Fog Mode
  GL_EXP  = $0800;
  GL_EXP2 = $0801;

// Front Face Direction
  GL_CW  = $0900;
  GL_CCW = $0901;

// Get Map Target
  GL_COEFF  = $0A00;
  GL_ORDER  = $0A01;
  GL_DOMAIN = $0A02;

// Get Pixel Map
  GL_PIXEL_MAP_I_TO_I = $0C70;
  GL_PIXEL_MAP_S_TO_S = $0C71;
  GL_PIXEL_MAP_I_TO_R = $0C72;
  GL_PIXEL_MAP_I_TO_G = $0C73;
  GL_PIXEL_MAP_I_TO_B = $0C74;
  GL_PIXEL_MAP_I_TO_A = $0C75;
  GL_PIXEL_MAP_R_TO_R = $0C76;
  GL_PIXEL_MAP_G_TO_G = $0C77;
  GL_PIXEL_MAP_B_TO_B = $0C78;
  GL_PIXEL_MAP_A_TO_A = $0C79;

// Get Target
  GL_CURRENT_COLOR                  = $0B00;
  GL_CURRENT_INDEX                  = $0B01;
  GL_CURRENT_NORMAL                 = $0B02;
  GL_CURRENT_TEXTURE_COORDS         = $0B03;
  GL_CURRENT_RASTER_COLOR           = $0B04;
  GL_CURRENT_RASTER_INDEX           = $0B05;
  GL_CURRENT_RASTER_TEXTURE_COORDS  = $0B06;
  GL_CURRENT_RASTER_POSITION        = $0B07;
  GL_CURRENT_RASTER_POSITION_VALID  = $0B08;
  GL_CURRENT_RASTER_DISTANCE        = $0B09;
  GL_POINT_SMOOTH                   = $0B10;
  GL_POINT_SIZE                     = $0B11;
  GL_POINT_SIZE_RANGE               = $0B12;
  GL_POINT_SIZE_GRANULARITY         = $0B13;
  GL_LINE_SMOOTH                    = $0B20;
  GL_LINE_WIDTH                     = $0B21;
  GL_LINE_WIDTH_RANGE               = $0B22;
  GL_LINE_WIDTH_GRANULARITY         = $0B23;
  GL_LINE_STIPPLE                   = $0B24;
  GL_LINE_STIPPLE_PATTERN           = $0B25;
  GL_LINE_STIPPLE_REPEAT            = $0B26;
  GL_LIST_MODE                      = $0B30;
  GL_MAX_LIST_NESTING               = $0B31;
  GL_LIST_BASE                      = $0B32;
  GL_LIST_INDEX                     = $0B33;
  GL_POLYGON_MODE                   = $0B40;
  GL_POLYGON_SMOOTH                 = $0B41;
  GL_POLYGON_STIPPLE                = $0B42;
  GL_EDGE_FLAG                      = $0B43;
  GL_CULL_FACE                      = $0B44;
  GL_CULL_FACE_MODE                 = $0B45;
  GL_FRONT_FACE                     = $0B46;
  GL_LIGHTING                       = $0B50;
  GL_LIGHT_MODEL_LOCAL_VIEWER       = $0B51;
  GL_LIGHT_MODEL_TWO_SIDE           = $0B52;
  GL_LIGHT_MODEL_AMBIENT            = $0B53;
  GL_SHADE_MODEL                    = $0B54;
  GL_COLOR_MATERIAL_FACE            = $0B55;
  GL_COLOR_MATERIAL_PARAMETER       = $0B56;
  GL_COLOR_MATERIAL                 = $0B57;
  GL_FOG                            = $0B60;
  GL_FOG_INDEX                      = $0B61;
  GL_FOG_DENSITY                    = $0B62;
  GL_FOG_START                      = $0B63;
  GL_FOG_END                        = $0B64;
  GL_FOG_MODE                       = $0B65;
  GL_FOG_COLOR                      = $0B66;
  GL_DEPTH_RANGE                    = $0B70;
  GL_DEPTH_TEST                     = $0B71;
  GL_DEPTH_WRITEMASK                = $0B72;
  GL_DEPTH_CLEAR_VALUE              = $0B73;
  GL_DEPTH_FUNC                     = $0B74;
  GL_ACCUM_CLEAR_VALUE              = $0B80;
  GL_STENCIL_TEST                   = $0B90;
  GL_STENCIL_CLEAR_VALUE            = $0B91;
  GL_STENCIL_FUNC                   = $0B92;
  GL_STENCIL_VALUE_MASK             = $0B93;
  GL_STENCIL_FAIL                   = $0B94;
  GL_STENCIL_PASS_DEPTH_FAIL        = $0B95;
  GL_STENCIL_PASS_DEPTH_PASS        = $0B96;
  GL_STENCIL_REF                    = $0B97;
  GL_STENCIL_WRITEMASK              = $0B98;
  GL_MATRIX_MODE                    = $0BA0;
  GL_NORMALIZE                      = $0BA1;
  GL_VIEWPORT                       = $0BA2;
  GL_MODELVIEW_STACK_DEPTH          = $0BA3;
  GL_PROJECTION_STACK_DEPTH         = $0BA4;
  GL_TEXTURE_STACK_DEPTH            = $0BA5;
  GL_MODELVIEW_MATRIX               = $0BA6;
  GL_PROJECTION_MATRIX              = $0BA7;
  GL_TEXTURE_MATRIX                 = $0BA8;
  GL_ATTRIB_STACK_DEPTH             = $0BB0;
  GL_ALPHA_TEST                     = $0BC0;
  GL_ALPHA_TEST_FUNC                = $0BC1;
  GL_ALPHA_TEST_REF                 = $0BC2;
  GL_DITHER                         = $0BD0;
  GL_BLEND_DST                      = $0BE0;
  GL_BLEND_SRC                      = $0BE1;
  GL_BLEND                          = $0BE2;
  GL_LOGIC_OP_MODE                  = $0BF0;
  GL_LOGIC_OP                       = $0BF1;
  GL_AUX_BUFFERS                    = $0C00;
  GL_DRAW_BUFFER                    = $0C01;
  GL_READ_BUFFER                    = $0C02;
  GL_SCISSOR_BOX                    = $0C10;
  GL_SCISSOR_TEST                   = $0C11;
  GL_INDEX_CLEAR_VALUE              = $0C20;
  GL_INDEX_WRITEMASK                = $0C21;
  GL_COLOR_CLEAR_VALUE              = $0C22;
  GL_COLOR_WRITEMASK                = $0C23;
  GL_INDEX_MODE                     = $0C30;
  GL_RGBA_MODE                      = $0C31;
  GL_DOUBLEBUFFER                   = $0C32;
  GL_STEREO                         = $0C33;
  GL_RENDER_MODE                    = $0C40;
  GL_PERSPECTIVE_CORRECTION_HINT    = $0C50;
  GL_POINT_SMOOTH_HINT              = $0C51;
  GL_LINE_SMOOTH_HINT               = $0C52;
  GL_POLYGON_SMOOTH_HINT            = $0C53;
  GL_FOG_HINT                       = $0C54;
  GL_TEXTURE_GEN_S                  = $0C60;
  GL_TEXTURE_GEN_T                  = $0C61;
  GL_TEXTURE_GEN_R                  = $0C62;
  GL_TEXTURE_GEN_Q                  = $0C63;
  GL_PIXEL_MAP_I_TO_I_SIZE          = $0CB0;
  GL_PIXEL_MAP_S_TO_S_SIZE          = $0CB1;
  GL_PIXEL_MAP_I_TO_R_SIZE          = $0CB2;
  GL_PIXEL_MAP_I_TO_G_SIZE          = $0CB3;
  GL_PIXEL_MAP_I_TO_B_SIZE          = $0CB4;
  GL_PIXEL_MAP_I_TO_A_SIZE          = $0CB5;
  GL_PIXEL_MAP_R_TO_R_SIZE          = $0CB6;
  GL_PIXEL_MAP_G_TO_G_SIZE          = $0CB7;
  GL_PIXEL_MAP_B_TO_B_SIZE          = $0CB8;
  GL_PIXEL_MAP_A_TO_A_SIZE          = $0CB9;
  GL_UNPACK_SWAP_BYTES              = $0CF0;
  GL_UNPACK_LSB_FIRST               = $0CF1;
  GL_UNPACK_ROW_LENGTH              = $0CF2;
  GL_UNPACK_SKIP_ROWS               = $0CF3;
  GL_UNPACK_SKIP_PIXELS             = $0CF4;
  GL_UNPACK_ALIGNMENT               = $0CF5;
  GL_PACK_SWAP_BYTES                = $0D00;
  GL_PACK_LSB_FIRST                 = $0D01;
  GL_PACK_ROW_LENGTH                = $0D02;
  GL_PACK_SKIP_ROWS                 = $0D03;
  GL_PACK_SKIP_PIXELS               = $0D04;
  GL_PACK_ALIGNMENT                 = $0D05;
  GL_MAP_COLOR                      = $0D10;
  GL_MAP_STENCIL                    = $0D11;
  GL_INDEX_SHIFT                    = $0D12;
  GL_INDEX_OFFSET                   = $0D13;
  GL_RED_SCALE                      = $0D14;
  GL_RED_BIAS                       = $0D15;
  GL_ZOOM_X                         = $0D16;
  GL_ZOOM_Y                         = $0D17;
  GL_GREEN_SCALE                    = $0D18;
  GL_GREEN_BIAS                     = $0D19;
  GL_BLUE_SCALE                     = $0D1A;
  GL_BLUE_BIAS                      = $0D1B;
  GL_ALPHA_SCALE                    = $0D1C;
  GL_ALPHA_BIAS                     = $0D1D;
  GL_DEPTH_SCALE                    = $0D1E;
  GL_DEPTH_BIAS                     = $0D1F;
  GL_MAX_EVAL_ORDER                 = $0D30;
  GL_MAX_LIGHTS                     = $0D31;
  GL_MAX_CLIP_PLANES                = $0D32;
  GL_MAX_TEXTURE_SIZE               = $0D33;
  GL_MAX_PIXEL_MAP_TABLE            = $0D34;
  GL_MAX_ATTRIB_STACK_DEPTH         = $0D35;
  GL_MAX_MODELVIEW_STACK_DEPTH      = $0D36;
  GL_MAX_NAME_STACK_DEPTH           = $0D37;
  GL_MAX_PROJECTION_STACK_DEPTH     = $0D38;
  GL_MAX_TEXTURE_STACK_DEPTH        = $0D39;
  GL_MAX_VIEWPORT_DIMS              = $0D3A;
  GL_SUBPIXEL_BITS                  = $0D50;
  GL_INDEX_BITS                     = $0D51;
  GL_RED_BITS                       = $0D52;
  GL_GREEN_BITS                     = $0D53;
  GL_BLUE_BITS                      = $0D54;
  GL_ALPHA_BITS                     = $0D55;
  GL_DEPTH_BITS                     = $0D56;
  GL_STENCIL_BITS                   = $0D57;
  GL_ACCUM_RED_BITS                 = $0D58;
  GL_ACCUM_GREEN_BITS               = $0D59;
  GL_ACCUM_BLUE_BITS                = $0D5A;
  GL_ACCUM_ALPHA_BITS               = $0D5B;
  GL_NAME_STACK_DEPTH               = $0D70;
  GL_AUTO_NORMAL                    = $0D80;
  GL_MAP1_COLOR_4                   = $0D90;
  GL_MAP1_INDEX                     = $0D91;
  GL_MAP1_NORMAL                    = $0D92;
  GL_MAP1_TEXTURE_COORD_1           = $0D93;
  GL_MAP1_TEXTURE_COORD_2           = $0D94;
  GL_MAP1_TEXTURE_COORD_3           = $0D95;
  GL_MAP1_TEXTURE_COORD_4           = $0D96;
  GL_MAP1_VERTEX_3                  = $0D97;
  GL_MAP1_VERTEX_4                  = $0D98;
  GL_MAP2_COLOR_4                   = $0DB0;
  GL_MAP2_INDEX                     = $0DB1;
  GL_MAP2_NORMAL                    = $0DB2;
  GL_MAP2_TEXTURE_COORD_1           = $0DB3;
  GL_MAP2_TEXTURE_COORD_2           = $0DB4;
  GL_MAP2_TEXTURE_COORD_3           = $0DB5;
  GL_MAP2_TEXTURE_COORD_4           = $0DB6;
  GL_MAP2_VERTEX_3                  = $0DB7;
  GL_MAP2_VERTEX_4                  = $0DB8;
  GL_MAP1_GRID_DOMAIN               = $0DD0;
  GL_MAP1_GRID_SEGMENTS             = $0DD1;
  GL_MAP2_GRID_DOMAIN               = $0DD2;
  GL_MAP2_GRID_SEGMENTS             = $0DD3;
  GL_TEXTURE_1D                     = $0DE0;
  GL_TEXTURE_2D                     = $0DE1;
  GL_TEXTURE_WIDTH                  = $1000;
  GL_TEXTURE_HEIGHT                 = $1001;
  GL_TEXTURE_COMPONENTS             = $1003;
  GL_TEXTURE_BORDER_COLOR           = $1004;
  GL_TEXTURE_BORDER                 = $1005;

// Hint Mode
  GL_DONT_CARE = $1100;
  GL_FASTEST   = $1101;
  GL_NICEST    = $1102;

// Light Parameter
  GL_AMBIENT               = $1200;
  GL_DIFFUSE               = $1201;
  GL_SPECULAR              = $1202;
  GL_POSITION              = $1203;
  GL_SPOT_DIRECTION        = $1204;
  GL_SPOT_EXPONENT         = $1205;
  GL_SPOT_CUTOFF           = $1206;
  GL_CONSTANT_ATTENUATION  = $1207;
  GL_LINEAR_ATTENUATION    = $1208;
  GL_QUADRATIC_ATTENUATION = $1209;

// List Mode
  GL_COMPILE             = $1300;
  GL_COMPILE_AND_EXECUTE = $1301;

// List Name Type
  GL_BYTE           = $1400;
  GL_UNSIGNED_BYTE  = $1401;
  GL_SHORT          = $1402;
  GL_UNSIGNED_SHORT = $1403;
  GL_INT            = $1404;
  GL_UNSIGNED_INT   = $1405;
  GL_FLOAT          = $1406;
  GL_2_BYTES        = $1407;
  GL_3_BYTES        = $1408;
  GL_4_BYTES        = $1409;

// Logic Op
  GL_CLEAR         = $1500;
  GL_AND           = $1501;
  GL_AND_REVERSE   = $1502;
  GL_COPY          = $1503;
  GL_AND_INVERTED  = $1504;
  GL_NOOP          = $1505;
  GL_XOR           = $1506;
  GL_OR            = $1507;
  GL_NOR           = $1508;
  GL_EQUIV         = $1509;
  GL_INVERT        = $150A;
  GL_OR_REVERSE    = $150B;
  GL_COPY_INVERTED = $150C;
  GL_OR_INVERTED   = $150D;
  GL_NAND          = $150E;
  GL_SET           = $150F;

// Material Parameter
  GL_EMISSION            = $1600;
  GL_SHININESS           = $1601;
  GL_AMBIENT_AND_DIFFUSE = $1602;
  GL_COLOR_INDEXES       = $1603;

// Matrix Mode
  GL_MODELVIEW  = $1700;
  GL_PROJECTION = $1701;
  GL_TEXTURE    = $1702;

// Pixel Copy Type
  GL_COLOR   = $1800;
  GL_DEPTH   = $1801;
  GL_STENCIL = $1802;

// Pixel Format
  GL_COLOR_INDEX     = $1900;
  GL_STENCIL_INDEX   = $1901;
  GL_DEPTH_COMPONENT = $1902;
  GL_RED             = $1903;
  GL_GREEN           = $1904;
  GL_BLUE            = $1905;
  GL_ALPHA           = $1906;
  GL_RGB             = $1907;
  GL_RGBA            = $1908;
  GL_LUMINANCE       = $1909;
  GL_LUMINANCE_ALPHA = $190A;

// Pixel Type
  GL_BITMAP = $1A00;

// Polygon Mode
  GL_POINT = $1B00;
  GL_LINE  = $1B01;
  GL_FILL  = $1B02;

// Rendering Mode
  GL_RENDER   = $1C00;
  GL_FEEDBACK = $1C01;
  GL_SELECT   = $1C02;

// Shading Model
  GL_FLAT   = $1D00;
  GL_SMOOTH = $1D01;

// Stencil Op
  GL_KEEP    = $1E00;
  GL_REPLACE = $1E01;
  GL_INCR    = $1E02;
  GL_DECR    = $1E03;

// String Name
  GL_VENDOR     = $1F00;
  GL_RENDERER   = $1F01;
  GL_VERSION    = $1F02;
  GL_EXTENSIONS = $1F03;

// Texture Coord Name
  GL_S = $2000;
  GL_T = $2001;
  GL_R = $2002;
  GL_Q = $2003;

// Texture Env Mode
  GL_MODULATE = $2100;
  GL_DECAL    = $2101;

// Texture Env Parameter
  GL_TEXTURE_ENV_MODE  = $2200;
  GL_TEXTURE_ENV_COLOR = $2201;

// Texture Env Target
  GL_TEXTURE_ENV = $2300;

// Texture Gen Mode
  GL_EYE_LINEAR    = $2400;
  GL_OBJECT_LINEAR = $2401;
  GL_SPHERE_MAP    = $2402;

// Texture Gen Parameter
  GL_TEXTURE_GEN_MODE = $2500;
  GL_OBJECT_PLANE     = $2501;
  GL_EYE_PLANE        = $2502;

// Texture Mag Filter
  GL_NEAREST = $2600;
  GL_LINEAR  = $2601;

// Texture Min Filter
  GL_NEAREST_MIPMAP_NEAREST = $2700;
  GL_LINEAR_MIPMAP_NEAREST  = $2701;
  GL_NEAREST_MIPMAP_LINEAR  = $2702;
  GL_LINEAR_MIPMAP_LINEAR   = $2703;

// Texture Parameter Name
  GL_TEXTURE_MAG_FILTER = $2800;
  GL_TEXTURE_MIN_FILTER = $2801;
  GL_TEXTURE_WRAP_S     = $2802;
  GL_TEXTURE_WRAP_T     = $2803;

// Texture Wrap Mode
  GL_CLAMP  = $2900;
  GL_REPEAT = $2901;

// Clip Plane Name
  GL_CLIP_PLANE0 = $3000;
  GL_CLIP_PLANE1 = $3001;
  GL_CLIP_PLANE2 = $3002;
  GL_CLIP_PLANE3 = $3003;
  GL_CLIP_PLANE4 = $3004;
  GL_CLIP_PLANE5 = $3005;

// Light Name
  GL_LIGHT0 = $4000;
  GL_LIGHT1 = $4001;
  GL_LIGHT2 = $4002;
  GL_LIGHT3 = $4003;
  GL_LIGHT4 = $4004;
  GL_LIGHT5 = $4005;
  GL_LIGHT6 = $4006;
  GL_LIGHT7 = $4007;

// Extensions
  GL_EXT_vertex_array = 1;
  GL_WIN_swap_hint    = 1;

// Ext Vertex Array
  GL_VERTEX_ARRAY_EXT                = $8074;
  GL_NORMAL_ARRAY_EXT                = $8075;
  GL_COLOR_ARRAY_EXT                 = $8076;
  GL_INDEX_ARRAY_EXT                 = $8077;
  GL_TEXTURE_COORD_ARRAY_EXT         = $8078;
  GL_EDGE_FLAG_ARRAY_EXT             = $8079;
  GL_VERTEX_ARRAY_SIZE_EXT           = $807A;
  GL_VERTEX_ARRAY_TYPE_EXT           = $807B;
  GL_VERTEX_ARRAY_STRIDE_EXT         = $807C;
  GL_VERTEX_ARRAY_COUNT_EXT          = $807D;
  GL_NORMAL_ARRAY_TYPE_EXT           = $807E;
  GL_NORMAL_ARRAY_STRIDE_EXT         = $807F;
  GL_NORMAL_ARRAY_COUNT_EXT          = $8080;
  GL_COLOR_ARRAY_SIZE_EXT            = $8081;
  GL_COLOR_ARRAY_TYPE_EXT            = $8082;
  GL_COLOR_ARRAY_STRIDE_EXT          = $8083;
  GL_COLOR_ARRAY_COUNT_EXT           = $8084;
  GL_INDEX_ARRAY_TYPE_EXT            = $8085;
  GL_INDEX_ARRAY_STRIDE_EXT          = $8086;
  GL_INDEX_ARRAY_COUNT_EXT           = $8087;
  GL_TEXTURE_COORD_ARRAY_SIZE_EXT    = $8088;
  GL_TEXTURE_COORD_ARRAY_TYPE_EXT    = $8089;
  GL_TEXTURE_COORD_ARRAY_STRIDE_EXT  = $808A;
  GL_TEXTURE_COORD_ARRAY_COUNT_EXT   = $808B;
  GL_EDGE_FLAG_ARRAY_STRIDE_EXT      = $808C;
  GL_EDGE_FLAG_ARRAY_COUNT_EXT       = $808D;
  GL_VERTEX_ARRAY_POINTER_EXT        = $808E;
  GL_NORMAL_ARRAY_POINTER_EXT        = $808F;
  GL_COLOR_ARRAY_POINTER_EXT         = $8090;
  GL_INDEX_ARRAY_POINTER_EXT         = $8091;
  GL_TEXTURE_COORD_ARRAY_POINTER_EXT = $8092;
  GL_EDGE_FLAG_ARRAY_POINTER_EXT     = $8093;

var
  kosglSwapBuffers:      procedure stdcall;
  kosglMakeCurrent:      function(X, Y: LongInt; SizeX, SizeY: LongInt; var CTX: TKOSGLContext): LongInt stdcall;

  glEnable:              procedure(Cap: GLEnum) stdcall;
  glDisable:             procedure(Cap: GLEnum) stdcall;

  glShadeModel:          procedure(Mode: GLEnum) stdcall;
  glCullFace:            procedure(Mode: GLEnum) stdcall;
  glPolygonMode:         procedure(Face, Mode: GLEnum) stdcall;

  glBegin:               procedure(Mode: GLEnum) stdcall;
  glEnd:                 procedure stdcall;

  glVertex2f:            procedure(X, Y: GLFloat) stdcall;
  glVertex2d:            procedure(X, Y: GLDouble) stdcall;
  glVertex2fv:           procedure(V: PGLFloat) stdcall;
  glVertex2dv:           procedure(V: PGLDouble) stdcall;
  glVertex3f:            procedure(X, Y, Z: GLFloat) stdcall;
  glVertex3d:            procedure(X, Y, Z: GLDouble) stdcall;
  glVertex3fv:           procedure(V: PGLFloat) stdcall;
  glVertex3dv:           procedure(V: PGLDouble) stdcall;
  glVertex4f:            procedure(X, Y, Z, W: GLFloat) stdcall;
  glVertex4d:            procedure(X, Y, Z, W: GLDouble) stdcall;
  glVertex4fv:           procedure(V: PGLFloat) stdcall;
  glVertex4dv:           procedure(V: PGLDouble) stdcall;

  glColor3f:             procedure(Red, Green, Blue: GLFloat) stdcall;
  glColor3d:             procedure(Red, Green, Blue: GLDouble) stdcall;
  glColor3fv:            procedure(V: PGLFloat) stdcall;
  glColor3dv:            procedure(V: PGLDouble) stdcall;
  glColor3ub:            procedure(Red, Green, Blue: GLUByte) stdcall;
  glColor4f:             procedure(Red, Green, Blue, Alpha: GLFloat) stdcall;
  glColor4d:             procedure(Red, Green, Blue, Alpha: GLDouble) stdcall;
  glColor4fv:            procedure(V: PGLFloat) stdcall;
  glColor4dv:            procedure(V: PGLDouble) stdcall;

  glNormal3f:            procedure(nX, nY, nZ: GLFloat) stdcall;
  glNormal3d:            procedure(nX, nY, nZ: GLDouble) stdcall;
  glNormal3fv:           procedure(V: PGLFloat) stdcall;
  glNormal3dv:           procedure(V: PGLDouble) stdcall;

  glTexCoord1f:          procedure(S: GLFloat) stdcall;
  glTexCoord1d:          procedure(S: GLDouble) stdcall;
  glTexCoord1fv:         procedure(V: PGLFloat) stdcall;
  glTexCoord1dv:         procedure(V: PGLDouble) stdcall;
  glTexCoord2f:          procedure(S, T: GLFloat) stdcall;
  glTexCoord2d:          procedure(S, T: GLDouble) stdcall;
  glTexCoord2fv:         procedure(V: PGLFloat) stdcall;
  glTexCoord2dv:         procedure(V: PGLDouble) stdcall;
  glTexCoord3f:          procedure(S, T, R: GLFloat) stdcall;
  glTexCoord3d:          procedure(S, T, R: GLDouble) stdcall;
  glTexCoord3fv:         procedure(V: PGLFloat) stdcall;
  glTexCoord3dv:         procedure(V: PGLDouble) stdcall;
  glTexCoord4f:          procedure(S, T, R, Q: GLFloat) stdcall;
  glTexCoord4d:          procedure(S, T, R, Q: GLDouble) stdcall;
  glTexCoord4fv:         procedure(V: PGLFloat) stdcall;
  glTexCoord4dv:         procedure(V: PGLDouble) stdcall;

  glEdgeFlag:            procedure(Flag: GLBoolean) stdcall;

  glMatrixMode:          procedure(Mode: GLEnum) stdcall;
  glLoadMatrixf:         procedure(M: PGLFloat) stdcall;
  glLoadIdentity:        procedure stdcall;
  glMultMatrixf:         procedure(M: PGLFloat) stdcall;
  glPushMatrix:          procedure stdcall;
  glPopMatrix:           procedure stdcall;
  glRotatef:             procedure(Angle, X, Y, Z: GLFloat) stdcall;
  glTranslatef:          procedure(X, Y, Z: GLFloat) stdcall;
  glScalef:              procedure(X, Y, Z: GLFloat) stdcall;

  glViewport:            procedure(X, Y: GLInt; Width, Height: GLSizei) stdcall;
  glFrustum:             procedure(Left, Right, Bottom, Top, zNear, zFar: GLDouble) stdcall;

  glGenLists:            function(Range: GLSizei): GLUInt stdcall;
  glIsList:              function(List: GLUInt): GLBoolean stdcall;
  glNewList:             procedure(ListIndex: GLUInt; Mode: GLEnum) stdcall;
  glEndList:             procedure stdcall;
  glCallList:            procedure(List: GLUInt) stdcall;

  glClear:               procedure(Mask: GLBitfield) stdcall;
  glClearColor:          procedure(Red, Green, Blue, Alpha: GLClampf) stdcall;
  glClearDepth:          procedure(Depth: GLClampd) stdcall;

  glRenderMode:          function(Mode: GLEnum): GLInt stdcall;
  glSelectBuffer:        procedure(Size: GLSizei; Buffer: PGLUInt) stdcall;

  glInitNames:           procedure stdcall;
  glPushName:            procedure(Name: GLUInt) stdcall;
  glPopName:             procedure stdcall;
  glLoadName:            procedure(Name: GLUInt) stdcall;

  glGenTextures:         procedure(N: GLSizei; var Textures: GLUInt) stdcall;
  glDeleteTextures:      procedure(N: GLSizei; Textures: PGLUInt) stdcall;
  glBindTexture:         procedure(Target: GLEnum; Texture: GLUInt) stdcall;
  glTexImage2D:          procedure(Target: GLEnum; Level, Components: GLInt; Width, Height: GLSizei; Border: GLInt; Format, _Type: GLEnum; Pixels: Pointer) stdcall;
  glTexEnvi:             procedure(Target, PName: GLEnum; Param: GLInt) stdcall;
  glTexParameteri:       procedure(Target, PName: GLEnum; Param: GLInt) stdcall;
  glPixelStorei:         procedure(PName: GLEnum; Param: GLInt) stdcall;

  glMaterialf:           procedure(Face, PName: GLEnum; Param: GLFloat) stdcall;
  glMaterialfv:          procedure(Face, PName: GLEnum; Params: PGLFloat) stdcall;
  glColorMaterial:       procedure(Face, Mode: GLEnum) stdcall;

  glLightfv:             procedure(Light, PName: GLEnum; Params: PGLFloat) stdcall;
  glLightf:              procedure(Light, PName: GLEnum; Param: GLFloat) stdcall;
  glLightModeli:         procedure(PName: GLEnum; Param: GLInt) stdcall;
  glLightModelfv:        procedure(PName: GLEnum; Params: PGLFloat) stdcall;

  glFlush:               procedure stdcall;
  glHint:                procedure(Target, Mode: GLEnum) stdcall;
  glGetIntegerv:         procedure(PName: GLenum; Params: PGLInt) stdcall;
  glGetFloatv:           procedure(PName: GLenum; Params: PGLFloat) stdcall;
  glFrontFace:           procedure(Mode: GLEnum) stdcall;

  glEnableClientState:   procedure(Cap: GLEnum) stdcall;
  glDisableClientState:  procedure(Cap: GLEnum) stdcall;
  glArrayElement:        procedure(Index: GLInt) stdcall;
  glDrawArrays:          procedure(Mode: GLEnum; First: GLInt; Count: GLSizei) stdcall;
  glDrawElements:        procedure(Mode: GLEnum; Count: GLSizei; _Type: GLEnum; const Indices: GLVoid) stdcall;
  glVertexPointer:       procedure(Size: GLInt; _Type: GLEnum; Stride: GLSizei; const Pointer: GLVoid) stdcall;
  glColorPointer:        procedure(Size: GLInt; _Type: GLEnum; Stride: GLSizei; const Pointer: GLVoid) stdcall;
  glNormalPointer:       procedure(_Type: GLEnum; Stride: GLSizei; const Pointer: GLVoid) stdcall;
  glTexCoordPointer:     procedure(Size: GLInt; _Type: GLEnum; Stride: GLSizei; const Pointer: GLVoid) stdcall;
  glPolygonOffset:       procedure(Factor: GLFloat; Units: GLFloat) stdcall;

  gluNewQuadric:         function: GLUquadricObj stdcall;
  gluDeleteQuadric:      procedure(State: GLUquadricObj) stdcall;
  gluQuadricDrawStyle:   procedure(QuadObject: GLUquadricObj; DrawStyle: GLEnum) stdcall;
  gluQuadricOrientation: procedure(QuadObject: GLUquadricObj; Orientation: GLEnum) stdcall;
  gluQuadricTexture:     procedure(QuadObject: GLUquadricObj; TextureCoords: GLBoolean ) stdcall;
  gluCylinder:           procedure(QuadObject: GLUquadricObj; BaseRadius, TopRadius, Height: GLDouble; Slices, Stacks: GLInt) stdcall;
  gluSphere:             procedure(QuadObject: GLUquadricObj; Radius: GLDouble; Slices, Loops: GLInt) stdcall;

  procedure TinyGL_initialization;
  
implementation

var
  hTinyGL: Pointer;

procedure TinyGL_initialization;
begin
  hTinyGL := LoadLibrary('/sys/lib/TinyGL.obj');
  kosglSwapBuffers      := GetProcAddress(hTinyGL, 'kosglSwapBuffers');
  kosglMakeCurrent      := GetProcAddress(hTinyGL, 'kosglMakeCurrent');
  glEnable              := GetProcAddress(hTinyGL, 'glEnable');
  glDisable             := GetProcAddress(hTinyGL, 'glDisable');
  glShadeModel          := GetProcAddress(hTinyGL, 'glShadeModel');
  glCullFace            := GetProcAddress(hTinyGL, 'glCullFace');
  glPolygonMode         := GetProcAddress(hTinyGL, 'glPolygonMode');
  glBegin               := GetProcAddress(hTinyGL, 'glBegin');
  glEnd                 := GetProcAddress(hTinyGL, 'glEnd');
  glVertex2f            := GetProcAddress(hTinyGL, 'glVertex2f');
  glVertex2d            := GetProcAddress(hTinyGL, 'glVertex2d');
  glVertex2fv           := GetProcAddress(hTinyGL, 'glVertex2fv');
  glVertex2dv           := GetProcAddress(hTinyGL, 'glVertex2dv');
  glVertex3f            := GetProcAddress(hTinyGL, 'glVertex3f');
  glVertex3d            := GetProcAddress(hTinyGL, 'glVertex3d');
  glVertex3fv           := GetProcAddress(hTinyGL, 'glVertex3fv');
  glVertex3dv           := GetProcAddress(hTinyGL, 'glVertex3dv');
  glVertex4f            := GetProcAddress(hTinyGL, 'glVertex4f');
  glVertex4d            := GetProcAddress(hTinyGL, 'glVertex4d');
  glVertex4fv           := GetProcAddress(hTinyGL, 'glVertex4fv');
  glVertex4dv           := GetProcAddress(hTinyGL, 'glVertex4dv');
  glColor3f             := GetProcAddress(hTinyGL, 'glColor3f');
  glColor3d             := GetProcAddress(hTinyGL, 'glColor3d');
  glColor3fv            := GetProcAddress(hTinyGL, 'glColor3fv');
  glColor3dv            := GetProcAddress(hTinyGL, 'glColor3dv');
  glColor3ub            := GetProcAddress(hTinyGL, 'glColor3ub');
  glColor4f             := GetProcAddress(hTinyGL, 'glColor4f');
  glColor4d             := GetProcAddress(hTinyGL, 'glColor4d');
  glColor4fv            := GetProcAddress(hTinyGL, 'glColor4fv');
  glColor4dv            := GetProcAddress(hTinyGL, 'glColor4dv');
  glNormal3f            := GetProcAddress(hTinyGL, 'glNormal3f');
  glNormal3d            := GetProcAddress(hTinyGL, 'glNormal3d');
  glNormal3fv           := GetProcAddress(hTinyGL, 'glNormal3fv');
  glNormal3dv           := GetProcAddress(hTinyGL, 'glNormal3dv');
  glTexCoord1f          := GetProcAddress(hTinyGL, 'glTexCoord1f');
  glTexCoord1d          := GetProcAddress(hTinyGL, 'glTexCoord1d');
  glTexCoord1fv         := GetProcAddress(hTinyGL, 'glTexCoord1fv');
  glTexCoord1dv         := GetProcAddress(hTinyGL, 'glTexCoord1dv');
  glTexCoord2f          := GetProcAddress(hTinyGL, 'glTexCoord2f');
  glTexCoord2d          := GetProcAddress(hTinyGL, 'glTexCoord2d');
  glTexCoord2fv         := GetProcAddress(hTinyGL, 'glTexCoord2fv');
  glTexCoord2dv         := GetProcAddress(hTinyGL, 'glTexCoord2dv');
  glTexCoord3f          := GetProcAddress(hTinyGL, 'glTexCoord3f');
  glTexCoord3d          := GetProcAddress(hTinyGL, 'glTexCoord3d');
  glTexCoord3fv         := GetProcAddress(hTinyGL, 'glTexCoord3fv');
  glTexCoord3dv         := GetProcAddress(hTinyGL, 'glTexCoord3dv');
  glTexCoord4f          := GetProcAddress(hTinyGL, 'glTexCoord4f');
  glTexCoord4d          := GetProcAddress(hTinyGL, 'glTexCoord4d');
  glTexCoord4fv         := GetProcAddress(hTinyGL, 'glTexCoord4fv');
  glTexCoord4dv         := GetProcAddress(hTinyGL, 'glTexCoord4dv');
  glEdgeFlag            := GetProcAddress(hTinyGL, 'glEdgeFlag');
  glMatrixMode          := GetProcAddress(hTinyGL, 'glMatrixMode');
  glLoadMatrixf         := GetProcAddress(hTinyGL, 'glLoadMatrixf');
  glLoadIdentity        := GetProcAddress(hTinyGL, 'glLoadIdentity');
  glMultMatrixf         := GetProcAddress(hTinyGL, 'glMultMatrixf');
  glPushMatrix          := GetProcAddress(hTinyGL, 'glPushMatrix');
  glPopMatrix           := GetProcAddress(hTinyGL, 'glPopMatrix');
  glRotatef             := GetProcAddress(hTinyGL, 'glRotatef');
  glTranslatef          := GetProcAddress(hTinyGL, 'glTranslatef');
  glScalef              := GetProcAddress(hTinyGL, 'glScalef');
  glViewport            := GetProcAddress(hTinyGL, 'glViewport');
  glFrustum             := GetProcAddress(hTinyGL, 'glFrustum');
  glGenLists            := GetProcAddress(hTinyGL, 'glGenLists');
  glIsList              := GetProcAddress(hTinyGL, 'glIsList');
  glNewList             := GetProcAddress(hTinyGL, 'glNewList');
  glEndList             := GetProcAddress(hTinyGL, 'glEndList');
  glCallList            := GetProcAddress(hTinyGL, 'glCallList');
  glClear               := GetProcAddress(hTinyGL, 'glClear');
  glClearColor          := GetProcAddress(hTinyGL, 'glClearColor');
  glClearDepth          := GetProcAddress(hTinyGL, 'glClearDepth');
  glRenderMode          := GetProcAddress(hTinyGL, 'glRenderMode');
  glSelectBuffer        := GetProcAddress(hTinyGL, 'glSelectBuffer');
  glInitNames           := GetProcAddress(hTinyGL, 'glInitNames');
  glPushName            := GetProcAddress(hTinyGL, 'glPushName');
  glPopName             := GetProcAddress(hTinyGL, 'glPopName');
  glLoadName            := GetProcAddress(hTinyGL, 'glLoadName');
  glGenTextures         := GetProcAddress(hTinyGL, 'glGenTextures');
  glDeleteTextures      := GetProcAddress(hTinyGL, 'glDeleteTextures');
  glBindTexture         := GetProcAddress(hTinyGL, 'glBindTexture');
  glTexImage2D          := GetProcAddress(hTinyGL, 'glTexImage2D');
  glTexEnvi             := GetProcAddress(hTinyGL, 'glTexEnvi');
  glTexParameteri       := GetProcAddress(hTinyGL, 'glTexParameteri');
  glPixelStorei         := GetProcAddress(hTinyGL, 'glPixelStorei');
  glMaterialf           := GetProcAddress(hTinyGL, 'glMaterialf');
  glMaterialfv          := GetProcAddress(hTinyGL, 'glMaterialfv');
  glColorMaterial       := GetProcAddress(hTinyGL, 'glColorMaterial');
  glLightfv             := GetProcAddress(hTinyGL, 'glLightfv');
  glLightf              := GetProcAddress(hTinyGL, 'glLightf');
  glLightModeli         := GetProcAddress(hTinyGL, 'glLightModeli');
  glLightModelfv        := GetProcAddress(hTinyGL, 'glLightModelfv');
  glFlush               := GetProcAddress(hTinyGL, 'glFlush');
  glHint                := GetProcAddress(hTinyGL, 'glHint');
  glGetIntegerv         := GetProcAddress(hTinyGL, 'glGetIntegerv');
  glGetFloatv           := GetProcAddress(hTinyGL, 'glGetFloatv');
  glFrontFace           := GetProcAddress(hTinyGL, 'glFrontFace');
  glEnableClientState   := GetProcAddress(hTinyGL, 'glEnableClientState');
  glDisableClientState  := GetProcAddress(hTinyGL, 'glDisableClientState');
  glArrayElement        := GetProcAddress(hTinyGL, 'glArrayElement');
  glDrawArrays          := GetProcAddress(hTinyGL, 'glDrawArrays');
  glDrawElements        := GetProcAddress(hTinyGL, 'glDrawElements');
  glVertexPointer       := GetProcAddress(hTinyGL, 'glVertexPointer');
  glColorPointer        := GetProcAddress(hTinyGL, 'glColorPointer');
  glNormalPointer       := GetProcAddress(hTinyGL, 'glNormalPointer');
  glTexCoordPointer     := GetProcAddress(hTinyGL, 'glTexCoordPointer');
  glPolygonOffset       := GetProcAddress(hTinyGL, 'glPolygonOffset');
  gluNewQuadric         := GetProcAddress(hTinyGL, 'gluNewQuadric');
  gluDeleteQuadric      := GetProcAddress(hTinyGL, 'gluDeleteQuadric');
  gluQuadricDrawStyle   := GetProcAddress(hTinyGL, 'gluQuadricDrawStyle');
  gluQuadricOrientation := GetProcAddress(hTinyGL, 'gluQuadricOrientation');
  gluQuadricTexture     := GetProcAddress(hTinyGL, 'gluQuadricTexture');
  gluCylinder           := GetProcAddress(hTinyGL, 'gluCylinder');
  gluSphere             := GetProcAddress(hTinyGL, 'gluSphere');
end;

end.