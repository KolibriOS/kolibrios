/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY__MESA_GLSL_GLSL_PARSER_H_INCLUDED
# define YY__MESA_GLSL_GLSL_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int _mesa_glsl_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ATTRIBUTE = 258,
    CONST_TOK = 259,
    BOOL_TOK = 260,
    FLOAT_TOK = 261,
    INT_TOK = 262,
    UINT_TOK = 263,
    DOUBLE_TOK = 264,
    BREAK = 265,
    CONTINUE = 266,
    DO = 267,
    ELSE = 268,
    FOR = 269,
    IF = 270,
    DISCARD = 271,
    RETURN = 272,
    SWITCH = 273,
    CASE = 274,
    DEFAULT = 275,
    BVEC2 = 276,
    BVEC3 = 277,
    BVEC4 = 278,
    IVEC2 = 279,
    IVEC3 = 280,
    IVEC4 = 281,
    UVEC2 = 282,
    UVEC3 = 283,
    UVEC4 = 284,
    VEC2 = 285,
    VEC3 = 286,
    VEC4 = 287,
    DVEC2 = 288,
    DVEC3 = 289,
    DVEC4 = 290,
    CENTROID = 291,
    IN_TOK = 292,
    OUT_TOK = 293,
    INOUT_TOK = 294,
    UNIFORM = 295,
    VARYING = 296,
    SAMPLE = 297,
    NOPERSPECTIVE = 298,
    FLAT = 299,
    SMOOTH = 300,
    MAT2X2 = 301,
    MAT2X3 = 302,
    MAT2X4 = 303,
    MAT3X2 = 304,
    MAT3X3 = 305,
    MAT3X4 = 306,
    MAT4X2 = 307,
    MAT4X3 = 308,
    MAT4X4 = 309,
    DMAT2X2 = 310,
    DMAT2X3 = 311,
    DMAT2X4 = 312,
    DMAT3X2 = 313,
    DMAT3X3 = 314,
    DMAT3X4 = 315,
    DMAT4X2 = 316,
    DMAT4X3 = 317,
    DMAT4X4 = 318,
    SAMPLER1D = 319,
    SAMPLER2D = 320,
    SAMPLER3D = 321,
    SAMPLERCUBE = 322,
    SAMPLER1DSHADOW = 323,
    SAMPLER2DSHADOW = 324,
    SAMPLERCUBESHADOW = 325,
    SAMPLER1DARRAY = 326,
    SAMPLER2DARRAY = 327,
    SAMPLER1DARRAYSHADOW = 328,
    SAMPLER2DARRAYSHADOW = 329,
    SAMPLERCUBEARRAY = 330,
    SAMPLERCUBEARRAYSHADOW = 331,
    ISAMPLER1D = 332,
    ISAMPLER2D = 333,
    ISAMPLER3D = 334,
    ISAMPLERCUBE = 335,
    ISAMPLER1DARRAY = 336,
    ISAMPLER2DARRAY = 337,
    ISAMPLERCUBEARRAY = 338,
    USAMPLER1D = 339,
    USAMPLER2D = 340,
    USAMPLER3D = 341,
    USAMPLERCUBE = 342,
    USAMPLER1DARRAY = 343,
    USAMPLER2DARRAY = 344,
    USAMPLERCUBEARRAY = 345,
    SAMPLER2DRECT = 346,
    ISAMPLER2DRECT = 347,
    USAMPLER2DRECT = 348,
    SAMPLER2DRECTSHADOW = 349,
    SAMPLERBUFFER = 350,
    ISAMPLERBUFFER = 351,
    USAMPLERBUFFER = 352,
    SAMPLER2DMS = 353,
    ISAMPLER2DMS = 354,
    USAMPLER2DMS = 355,
    SAMPLER2DMSARRAY = 356,
    ISAMPLER2DMSARRAY = 357,
    USAMPLER2DMSARRAY = 358,
    SAMPLEREXTERNALOES = 359,
    IMAGE1D = 360,
    IMAGE2D = 361,
    IMAGE3D = 362,
    IMAGE2DRECT = 363,
    IMAGECUBE = 364,
    IMAGEBUFFER = 365,
    IMAGE1DARRAY = 366,
    IMAGE2DARRAY = 367,
    IMAGECUBEARRAY = 368,
    IMAGE2DMS = 369,
    IMAGE2DMSARRAY = 370,
    IIMAGE1D = 371,
    IIMAGE2D = 372,
    IIMAGE3D = 373,
    IIMAGE2DRECT = 374,
    IIMAGECUBE = 375,
    IIMAGEBUFFER = 376,
    IIMAGE1DARRAY = 377,
    IIMAGE2DARRAY = 378,
    IIMAGECUBEARRAY = 379,
    IIMAGE2DMS = 380,
    IIMAGE2DMSARRAY = 381,
    UIMAGE1D = 382,
    UIMAGE2D = 383,
    UIMAGE3D = 384,
    UIMAGE2DRECT = 385,
    UIMAGECUBE = 386,
    UIMAGEBUFFER = 387,
    UIMAGE1DARRAY = 388,
    UIMAGE2DARRAY = 389,
    UIMAGECUBEARRAY = 390,
    UIMAGE2DMS = 391,
    UIMAGE2DMSARRAY = 392,
    IMAGE1DSHADOW = 393,
    IMAGE2DSHADOW = 394,
    IMAGE1DARRAYSHADOW = 395,
    IMAGE2DARRAYSHADOW = 396,
    COHERENT = 397,
    VOLATILE = 398,
    RESTRICT = 399,
    READONLY = 400,
    WRITEONLY = 401,
    ATOMIC_UINT = 402,
    STRUCT = 403,
    VOID_TOK = 404,
    WHILE = 405,
    IDENTIFIER = 406,
    TYPE_IDENTIFIER = 407,
    NEW_IDENTIFIER = 408,
    FLOATCONSTANT = 409,
    DOUBLECONSTANT = 410,
    INTCONSTANT = 411,
    UINTCONSTANT = 412,
    BOOLCONSTANT = 413,
    FIELD_SELECTION = 414,
    LEFT_OP = 415,
    RIGHT_OP = 416,
    INC_OP = 417,
    DEC_OP = 418,
    LE_OP = 419,
    GE_OP = 420,
    EQ_OP = 421,
    NE_OP = 422,
    AND_OP = 423,
    OR_OP = 424,
    XOR_OP = 425,
    MUL_ASSIGN = 426,
    DIV_ASSIGN = 427,
    ADD_ASSIGN = 428,
    MOD_ASSIGN = 429,
    LEFT_ASSIGN = 430,
    RIGHT_ASSIGN = 431,
    AND_ASSIGN = 432,
    XOR_ASSIGN = 433,
    OR_ASSIGN = 434,
    SUB_ASSIGN = 435,
    INVARIANT = 436,
    PRECISE = 437,
    LOWP = 438,
    MEDIUMP = 439,
    HIGHP = 440,
    SUPERP = 441,
    PRECISION = 442,
    VERSION_TOK = 443,
    EXTENSION = 444,
    LINE = 445,
    COLON = 446,
    EOL = 447,
    INTERFACE = 448,
    OUTPUT = 449,
    PRAGMA_DEBUG_ON = 450,
    PRAGMA_DEBUG_OFF = 451,
    PRAGMA_OPTIMIZE_ON = 452,
    PRAGMA_OPTIMIZE_OFF = 453,
    PRAGMA_INVARIANT_ALL = 454,
    LAYOUT_TOK = 455,
    ASM = 456,
    CLASS = 457,
    UNION = 458,
    ENUM = 459,
    TYPEDEF = 460,
    TEMPLATE = 461,
    THIS = 462,
    PACKED_TOK = 463,
    GOTO = 464,
    INLINE_TOK = 465,
    NOINLINE = 466,
    PUBLIC_TOK = 467,
    STATIC = 468,
    EXTERN = 469,
    EXTERNAL = 470,
    LONG_TOK = 471,
    SHORT_TOK = 472,
    HALF = 473,
    FIXED_TOK = 474,
    UNSIGNED = 475,
    INPUT_TOK = 476,
    HVEC2 = 477,
    HVEC3 = 478,
    HVEC4 = 479,
    FVEC2 = 480,
    FVEC3 = 481,
    FVEC4 = 482,
    SAMPLER3DRECT = 483,
    SIZEOF = 484,
    CAST = 485,
    NAMESPACE = 486,
    USING = 487,
    RESOURCE = 488,
    PATCH = 489,
    SUBROUTINE = 490,
    ERROR_TOK = 491,
    COMMON = 492,
    PARTITION = 493,
    ACTIVE = 494,
    FILTER = 495,
    ROW_MAJOR = 496,
    THEN = 497
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 98 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1909  */

   int n;
   float real;
   double dreal;
   const char *identifier;

   struct ast_type_qualifier type_qualifier;

   ast_node *node;
   ast_type_specifier *type_specifier;
   ast_array_specifier *array_specifier;
   ast_fully_specified_type *fully_specified_type;
   ast_function *function;
   ast_parameter_declarator *parameter_declarator;
   ast_function_definition *function_definition;
   ast_compound_statement *compound_statement;
   ast_expression *expression;
   ast_declarator_list *declarator_list;
   ast_struct_specifier *struct_specifier;
   ast_declaration *declaration;
   ast_switch_body *switch_body;
   ast_case_label *case_label;
   ast_case_label_list *case_label_list;
   ast_case_statement *case_statement;
   ast_case_statement_list *case_statement_list;
   ast_interface_block *interface_block;

   struct {
      ast_node *cond;
      ast_expression *rest;
   } for_rest_statement;

   struct {
      ast_node *then_statement;
      ast_node *else_statement;
   } selection_rest_statement;

#line 335 "./glsl_parser.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int _mesa_glsl_parse (struct _mesa_glsl_parse_state *state);

#endif /* !YY__MESA_GLSL_GLSL_PARSER_H_INCLUDED  */
