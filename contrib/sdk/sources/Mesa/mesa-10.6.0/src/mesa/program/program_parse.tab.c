/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         _mesa_program_parse
#define yylex           _mesa_program_lex
#define yyerror         _mesa_program_error
#define yydebug         _mesa_program_debug
#define yynerrs         _mesa_program_nerrs


/* Copy the first part of user declarations.  */
#line 1 "../../../src/mesa/program/program_parse.y" /* yacc.c:339  */

/*
 * Copyright © 2009 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main/mtypes.h"
#include "main/imports.h"
#include "program/program.h"
#include "program/prog_parameter.h"
#include "program/prog_parameter_layout.h"
#include "program/prog_statevars.h"
#include "program/prog_instruction.h"

#include "program/symbol_table.h"
#include "program/program_parser.h"

extern void *yy_scan_string(char *);
extern void yy_delete_buffer(void *);

static struct asm_symbol *declare_variable(struct asm_parser_state *state,
    char *name, enum asm_type t, struct YYLTYPE *locp);

static int add_state_reference(struct gl_program_parameter_list *param_list,
    const gl_state_index tokens[STATE_LENGTH]);

static int initialize_symbol_from_state(struct gl_program *prog,
    struct asm_symbol *param_var, const gl_state_index tokens[STATE_LENGTH]);

static int initialize_symbol_from_param(struct gl_program *prog,
    struct asm_symbol *param_var, const gl_state_index tokens[STATE_LENGTH]);

static int initialize_symbol_from_const(struct gl_program *prog,
    struct asm_symbol *param_var, const struct asm_vector *vec,
    GLboolean allowSwizzle);

static int yyparse(struct asm_parser_state *state);

static char *make_error_string(const char *fmt, ...);

static void yyerror(struct YYLTYPE *locp, struct asm_parser_state *state,
    const char *s);

static int validate_inputs(struct YYLTYPE *locp,
    struct asm_parser_state *state);

static void init_dst_reg(struct prog_dst_register *r);

static void set_dst_reg(struct prog_dst_register *r,
                        gl_register_file file, GLint index);

static void init_src_reg(struct asm_src_register *r);

static void set_src_reg(struct asm_src_register *r,
                        gl_register_file file, GLint index);

static void set_src_reg_swz(struct asm_src_register *r,
                            gl_register_file file, GLint index, GLuint swizzle);

static void asm_instruction_set_operands(struct asm_instruction *inst,
    const struct prog_dst_register *dst, const struct asm_src_register *src0,
    const struct asm_src_register *src1, const struct asm_src_register *src2);

static struct asm_instruction *asm_instruction_ctor(gl_inst_opcode op,
    const struct prog_dst_register *dst, const struct asm_src_register *src0,
    const struct asm_src_register *src1, const struct asm_src_register *src2);

static struct asm_instruction *asm_instruction_copy_ctor(
    const struct prog_instruction *base, const struct prog_dst_register *dst,
    const struct asm_src_register *src0, const struct asm_src_register *src1,
    const struct asm_src_register *src2);

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#define YYLLOC_DEFAULT(Current, Rhs, N)					\
   do {									\
      if (N) {							\
	 (Current).first_line = YYRHSLOC(Rhs, 1).first_line;		\
	 (Current).first_column = YYRHSLOC(Rhs, 1).first_column;	\
	 (Current).position = YYRHSLOC(Rhs, 1).position;		\
	 (Current).last_line = YYRHSLOC(Rhs, N).last_line;		\
	 (Current).last_column = YYRHSLOC(Rhs, N).last_column;		\
      } else {								\
	 (Current).first_line = YYRHSLOC(Rhs, 0).last_line;		\
	 (Current).last_line = (Current).first_line;			\
	 (Current).first_column = YYRHSLOC(Rhs, 0).last_column;		\
	 (Current).last_column = (Current).first_column;		\
	 (Current).position = YYRHSLOC(Rhs, 0).position			\
	    + (Current).first_column;					\
      }									\
   } while(0)

#line 191 "program/program_parse.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "program_parse.tab.h".  */
#ifndef YY__MESA_PROGRAM_PROGRAM_PROGRAM_PARSE_TAB_H_INCLUDED
# define YY__MESA_PROGRAM_PROGRAM_PROGRAM_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int _mesa_program_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ARBvp_10 = 258,
    ARBfp_10 = 259,
    ADDRESS = 260,
    ALIAS = 261,
    ATTRIB = 262,
    OPTION = 263,
    OUTPUT = 264,
    PARAM = 265,
    TEMP = 266,
    END = 267,
    BIN_OP = 268,
    BINSC_OP = 269,
    SAMPLE_OP = 270,
    SCALAR_OP = 271,
    TRI_OP = 272,
    VECTOR_OP = 273,
    ARL = 274,
    KIL = 275,
    SWZ = 276,
    TXD_OP = 277,
    INTEGER = 278,
    REAL = 279,
    AMBIENT = 280,
    ATTENUATION = 281,
    BACK = 282,
    CLIP = 283,
    COLOR = 284,
    DEPTH = 285,
    DIFFUSE = 286,
    DIRECTION = 287,
    EMISSION = 288,
    ENV = 289,
    EYE = 290,
    FOG = 291,
    FOGCOORD = 292,
    FRAGMENT = 293,
    FRONT = 294,
    HALF = 295,
    INVERSE = 296,
    INVTRANS = 297,
    LIGHT = 298,
    LIGHTMODEL = 299,
    LIGHTPROD = 300,
    LOCAL = 301,
    MATERIAL = 302,
    MAT_PROGRAM = 303,
    MATRIX = 304,
    MATRIXINDEX = 305,
    MODELVIEW = 306,
    MVP = 307,
    NORMAL = 308,
    OBJECT = 309,
    PALETTE = 310,
    PARAMS = 311,
    PLANE = 312,
    POINT_TOK = 313,
    POINTSIZE = 314,
    POSITION = 315,
    PRIMARY = 316,
    PROGRAM = 317,
    PROJECTION = 318,
    RANGE = 319,
    RESULT = 320,
    ROW = 321,
    SCENECOLOR = 322,
    SECONDARY = 323,
    SHININESS = 324,
    SIZE_TOK = 325,
    SPECULAR = 326,
    SPOT = 327,
    STATE = 328,
    TEXCOORD = 329,
    TEXENV = 330,
    TEXGEN = 331,
    TEXGEN_Q = 332,
    TEXGEN_R = 333,
    TEXGEN_S = 334,
    TEXGEN_T = 335,
    TEXTURE = 336,
    TRANSPOSE = 337,
    TEXTURE_UNIT = 338,
    TEX_1D = 339,
    TEX_2D = 340,
    TEX_3D = 341,
    TEX_CUBE = 342,
    TEX_RECT = 343,
    TEX_SHADOW1D = 344,
    TEX_SHADOW2D = 345,
    TEX_SHADOWRECT = 346,
    TEX_ARRAY1D = 347,
    TEX_ARRAY2D = 348,
    TEX_ARRAYSHADOW1D = 349,
    TEX_ARRAYSHADOW2D = 350,
    VERTEX = 351,
    VTXATTRIB = 352,
    WEIGHT = 353,
    IDENTIFIER = 354,
    USED_IDENTIFIER = 355,
    MASK4 = 356,
    MASK3 = 357,
    MASK2 = 358,
    MASK1 = 359,
    SWIZZLE = 360,
    DOT_DOT = 361,
    DOT = 362
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 126 "../../../src/mesa/program/program_parse.y" /* yacc.c:355  */

   struct asm_instruction *inst;
   struct asm_symbol *sym;
   struct asm_symbol temp_sym;
   struct asm_swizzle_mask swiz_mask;
   struct asm_src_register src_reg;
   struct prog_dst_register dst_reg;
   struct prog_instruction temp_inst;
   char *string;
   unsigned result;
   unsigned attrib;
   int integer;
   float real;
   gl_state_index state[STATE_LENGTH];
   int negate;
   struct asm_vector vector;
   gl_inst_opcode opcode;

   struct {
      unsigned swz;
      unsigned rgba_valid:1;
      unsigned xyzw_valid:1;
      unsigned negate:1;
   } ext_swizzle;

#line 365 "program/program_parse.tab.c" /* yacc.c:355  */
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



int _mesa_program_parse (struct asm_parser_state *state);

#endif /* !YY__MESA_PROGRAM_PROGRAM_PROGRAM_PARSE_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */
#line 271 "../../../src/mesa/program/program_parse.y" /* yacc.c:358  */

extern int
_mesa_program_lexer_lex(YYSTYPE *yylval_param, YYLTYPE *yylloc_param,
                        void *yyscanner);

static int
yylex(YYSTYPE *yylval_param, YYLTYPE *yylloc_param,
      struct asm_parser_state *state)
{
   return _mesa_program_lexer_lex(yylval_param, yylloc_param, state->scanner);
}

#line 407 "program/program_parse.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   402

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  120
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  143
/* YYNRULES -- Number of rules.  */
#define YYNRULES  283
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  478

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   362

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     115,   116,     2,   113,   109,   114,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   108,
       2,   117,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   111,     2,   112,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   118,   110,   119,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   286,   286,   289,   297,   309,   310,   313,   337,   338,
     341,   356,   359,   364,   371,   372,   373,   374,   375,   376,
     377,   380,   381,   382,   385,   391,   399,   405,   412,   418,
     425,   469,   474,   483,   527,   533,   534,   535,   536,   537,
     538,   539,   540,   541,   542,   543,   544,   547,   559,   567,
     584,   591,   610,   621,   641,   665,   672,   705,   712,   728,
     787,   830,   839,   861,   871,   875,   904,   923,   923,   925,
     932,   944,   945,   946,   949,   963,   977,   997,  1008,  1020,
    1022,  1023,  1024,  1025,  1028,  1028,  1028,  1028,  1029,  1032,
    1036,  1041,  1047,  1054,  1061,  1083,  1105,  1106,  1107,  1108,
    1109,  1110,  1113,  1132,  1136,  1142,  1146,  1150,  1154,  1158,
    1162,  1166,  1171,  1177,  1188,  1188,  1189,  1191,  1195,  1199,
    1203,  1209,  1209,  1211,  1229,  1255,  1258,  1273,  1279,  1285,
    1286,  1293,  1299,  1305,  1313,  1319,  1325,  1333,  1339,  1345,
    1353,  1354,  1357,  1358,  1359,  1360,  1361,  1362,  1363,  1364,
    1365,  1366,  1367,  1370,  1379,  1383,  1387,  1393,  1402,  1406,
    1410,  1419,  1423,  1429,  1435,  1442,  1447,  1455,  1465,  1467,
    1475,  1481,  1485,  1489,  1495,  1506,  1515,  1519,  1524,  1528,
    1532,  1536,  1542,  1549,  1553,  1559,  1567,  1578,  1585,  1589,
    1595,  1605,  1616,  1620,  1638,  1647,  1650,  1656,  1660,  1664,
    1670,  1681,  1686,  1691,  1696,  1701,  1706,  1714,  1717,  1722,
    1735,  1743,  1754,  1762,  1762,  1764,  1764,  1766,  1776,  1781,
    1788,  1798,  1807,  1812,  1819,  1829,  1839,  1851,  1851,  1852,
    1852,  1854,  1864,  1872,  1882,  1890,  1898,  1907,  1918,  1922,
    1928,  1929,  1930,  1933,  1933,  1936,  1971,  1975,  1975,  1978,
    1985,  1994,  2008,  2017,  2026,  2030,  2039,  2048,  2059,  2066,
    2076,  2104,  2113,  2125,  2128,  2137,  2148,  2149,  2150,  2153,
    2154,  2155,  2158,  2159,  2162,  2163,  2166,  2167,  2170,  2181,
    2192,  2203,  2229,  2230
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ARBvp_10", "ARBfp_10", "ADDRESS",
  "ALIAS", "ATTRIB", "OPTION", "OUTPUT", "PARAM", "TEMP", "END", "BIN_OP",
  "BINSC_OP", "SAMPLE_OP", "SCALAR_OP", "TRI_OP", "VECTOR_OP", "ARL",
  "KIL", "SWZ", "TXD_OP", "INTEGER", "REAL", "AMBIENT", "ATTENUATION",
  "BACK", "CLIP", "COLOR", "DEPTH", "DIFFUSE", "DIRECTION", "EMISSION",
  "ENV", "EYE", "FOG", "FOGCOORD", "FRAGMENT", "FRONT", "HALF", "INVERSE",
  "INVTRANS", "LIGHT", "LIGHTMODEL", "LIGHTPROD", "LOCAL", "MATERIAL",
  "MAT_PROGRAM", "MATRIX", "MATRIXINDEX", "MODELVIEW", "MVP", "NORMAL",
  "OBJECT", "PALETTE", "PARAMS", "PLANE", "POINT_TOK", "POINTSIZE",
  "POSITION", "PRIMARY", "PROGRAM", "PROJECTION", "RANGE", "RESULT", "ROW",
  "SCENECOLOR", "SECONDARY", "SHININESS", "SIZE_TOK", "SPECULAR", "SPOT",
  "STATE", "TEXCOORD", "TEXENV", "TEXGEN", "TEXGEN_Q", "TEXGEN_R",
  "TEXGEN_S", "TEXGEN_T", "TEXTURE", "TRANSPOSE", "TEXTURE_UNIT", "TEX_1D",
  "TEX_2D", "TEX_3D", "TEX_CUBE", "TEX_RECT", "TEX_SHADOW1D",
  "TEX_SHADOW2D", "TEX_SHADOWRECT", "TEX_ARRAY1D", "TEX_ARRAY2D",
  "TEX_ARRAYSHADOW1D", "TEX_ARRAYSHADOW2D", "VERTEX", "VTXATTRIB",
  "WEIGHT", "IDENTIFIER", "USED_IDENTIFIER", "MASK4", "MASK3", "MASK2",
  "MASK1", "SWIZZLE", "DOT_DOT", "DOT", "';'", "','", "'|'", "'['", "']'",
  "'+'", "'-'", "'('", "')'", "'='", "'{'", "'}'", "$accept", "program",
  "language", "optionSequence", "option", "statementSequence", "statement",
  "instruction", "ALU_instruction", "TexInstruction", "ARL_instruction",
  "VECTORop_instruction", "SCALARop_instruction", "BINSCop_instruction",
  "BINop_instruction", "TRIop_instruction", "SAMPLE_instruction",
  "KIL_instruction", "TXD_instruction", "texImageUnit", "texTarget",
  "SWZ_instruction", "scalarSrcReg", "scalarUse", "swizzleSrcReg",
  "maskedDstReg", "maskedAddrReg", "extendedSwizzle", "extSwizComp",
  "extSwizSel", "srcReg", "dstReg", "progParamArray", "progParamArrayMem",
  "progParamArrayAbs", "progParamArrayRel", "addrRegRelOffset",
  "addrRegPosOffset", "addrRegNegOffset", "addrReg", "addrComponent",
  "addrWriteMask", "scalarSuffix", "swizzleSuffix", "optionalMask",
  "optionalCcMask", "ccTest", "ccTest2", "ccMaskRule", "ccMaskRule2",
  "namingStatement", "ATTRIB_statement", "attribBinding", "vtxAttribItem",
  "vtxAttribNum", "vtxOptWeightNum", "vtxWeightNum", "fragAttribItem",
  "PARAM_statement", "PARAM_singleStmt", "PARAM_multipleStmt",
  "optArraySize", "paramSingleInit", "paramMultipleInit",
  "paramMultInitList", "paramSingleItemDecl", "paramSingleItemUse",
  "paramMultipleItem", "stateMultipleItem", "stateSingleItem",
  "stateMaterialItem", "stateMatProperty", "stateLightItem",
  "stateLightProperty", "stateSpotProperty", "stateLightModelItem",
  "stateLModProperty", "stateLightProdItem", "stateLProdProperty",
  "stateTexEnvItem", "stateTexEnvProperty", "ambDiffSpecProperty",
  "stateLightNumber", "stateTexGenItem", "stateTexGenType",
  "stateTexGenCoord", "stateFogItem", "stateFogProperty",
  "stateClipPlaneItem", "stateClipPlaneNum", "statePointItem",
  "statePointProperty", "stateMatrixRow", "stateMatrixRows",
  "optMatrixRows", "stateMatrixItem", "stateOptMatModifier",
  "stateMatModifier", "stateMatrixRowNum", "stateMatrixName",
  "stateOptModMatNum", "stateModMatNum", "statePaletteMatNum",
  "stateProgramMatNum", "stateDepthItem", "programSingleItem",
  "programMultipleItem", "progEnvParams", "progEnvParamNums",
  "progEnvParam", "progLocalParams", "progLocalParamNums",
  "progLocalParam", "progEnvParamNum", "progLocalParamNum",
  "paramConstDecl", "paramConstUse", "paramConstScalarDecl",
  "paramConstScalarUse", "paramConstVector", "signedFloatConstant",
  "optionalSign", "TEMP_statement", "@1", "optVarSize",
  "ADDRESS_statement", "@2", "varNameList", "OUTPUT_statement",
  "resultBinding", "resultColBinding", "optResultFaceType",
  "optResultColorType", "optFaceType", "optColorType",
  "optTexCoordUnitNum", "optTexImageUnitNum", "optLegacyTexUnitNum",
  "texCoordUnitNum", "texImageUnitNum", "legacyTexUnitNum",
  "ALIAS_statement", "string", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,    59,    44,
     124,    91,    93,    43,    45,    40,    41,    61,   123,   125
};
# endif

#define YYPACT_NINF -398

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-398)))

#define YYTABLE_NINF -230

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      52,  -398,  -398,    14,  -398,  -398,    67,   152,  -398,    24,
    -398,  -398,     5,  -398,    47,    81,    99,  -398,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    56,    -1,    -1,  -398,    97,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,  -398,   112,  -398,  -398,  -398,  -398,  -398,   156,  -398,
    -398,  -398,  -398,  -398,   111,    98,   141,    95,   127,  -398,
      84,   142,  -398,   146,   150,   153,   157,   158,  -398,   159,
     165,  -398,  -398,  -398,  -398,  -398,   113,   -13,   161,   163,
    -398,  -398,   162,  -398,  -398,   164,   174,    10,   252,    -3,
    -398,   -11,  -398,  -398,  -398,  -398,   166,  -398,   -20,  -398,
    -398,  -398,  -398,   167,   -20,   -20,   -20,   -20,   -20,   -20,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,   137,    70,
     132,    85,   168,    34,   -20,   113,   169,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,  -398,    34,   -20,   171,   111,
     179,  -398,  -398,  -398,   172,  -398,  -398,  -398,  -398,  -398,
    -398,  -398,   216,  -398,  -398,   253,    76,   258,  -398,   176,
     154,  -398,   178,    29,   180,  -398,   181,  -398,  -398,   110,
    -398,  -398,   166,  -398,   175,   182,   183,   219,    32,   184,
     177,   186,    94,   140,     7,   187,   166,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,   226,  -398,   110,
    -398,   188,  -398,  -398,   166,   189,   190,  -398,   113,     9,
    -398,     1,   193,   195,   240,   164,  -398,   191,  -398,  -398,
     194,  -398,  -398,  -398,  -398,   197,   -20,  -398,   196,   198,
     113,   -20,    34,  -398,   203,   206,   228,   -20,  -398,  -398,
    -398,  -398,   290,   292,   293,  -398,  -398,  -398,  -398,   294,
    -398,  -398,  -398,  -398,   251,   294,    48,   208,   209,  -398,
     210,  -398,   166,    21,  -398,  -398,  -398,   299,   295,    12,
     212,  -398,   302,  -398,   304,   302,  -398,   218,   -20,  -398,
    -398,   217,  -398,  -398,   227,   -20,   -20,  -398,   214,  -398,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,   220,  -398,  -398,
     222,   225,   229,  -398,   223,  -398,   224,  -398,   230,  -398,
     231,  -398,   233,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
     314,   316,  -398,   317,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,   234,  -398,  -398,  -398,  -398,   170,   318,  -398,   235,
    -398,   236,   237,  -398,    44,  -398,  -398,   143,  -398,   244,
     -15,   245,    36,  -398,   332,  -398,   138,   -20,  -398,  -398,
     301,   101,    94,  -398,   248,  -398,   249,  -398,   250,  -398,
    -398,  -398,  -398,  -398,  -398,  -398,   254,  -398,  -398,  -398,
     -20,  -398,   333,   340,  -398,   -20,  -398,  -398,  -398,   -20,
     102,   132,    75,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,   255,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
     336,  -398,  -398,    49,  -398,  -398,  -398,  -398,    90,  -398,
    -398,  -398,  -398,   256,   260,   259,   261,  -398,   298,    36,
    -398,  -398,  -398,  -398,  -398,  -398,   -20,  -398,   -20,   228,
     290,   292,   262,  -398,  -398,   257,   265,   268,   266,   273,
     269,   274,   318,  -398,   -20,   138,  -398,   290,  -398,   292,
     107,  -398,  -398,  -398,  -398,   318,   270,  -398
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     3,     4,     0,     6,     1,     9,     0,     5,   246,
     282,   283,     0,   247,     0,     0,     0,     2,     0,     0,
       0,     0,     0,     0,     0,   242,     0,     0,     8,     0,
      12,    13,    14,    15,    16,    17,    18,    19,    21,    22,
      23,    20,     0,    96,    97,   121,   122,    98,     0,    99,
     100,   101,   245,     7,     0,     0,     0,     0,     0,    65,
       0,    88,    64,     0,     0,     0,     0,     0,    76,     0,
       0,    94,   240,   241,    31,    32,    83,     0,     0,     0,
      10,    11,     0,   243,   250,   248,     0,     0,   125,   242,
     123,   259,   257,   253,   255,   252,   272,   254,   242,    84,
      85,    86,    87,    91,   242,   242,   242,   242,   242,   242,
      78,    55,    81,    80,    82,    92,   233,   232,     0,     0,
       0,     0,    60,     0,   242,    83,     0,    61,    63,   134,
     135,   213,   214,   136,   229,   230,     0,   242,     0,     0,
       0,   281,   102,   126,     0,   127,   131,   132,   133,   227,
     228,   231,     0,   262,   261,     0,   263,     0,   256,     0,
       0,    54,     0,     0,     0,    26,     0,    25,    24,   269,
     119,   117,   272,   104,     0,     0,     0,     0,     0,     0,
     266,     0,   266,     0,     0,   276,   272,   142,   143,   144,
     145,   147,   146,   148,   149,   150,   151,     0,   152,   269,
     109,     0,   107,   105,   272,     0,   114,   103,    83,     0,
      52,     0,     0,     0,     0,   244,   249,     0,   239,   238,
       0,   264,   265,   258,   278,     0,   242,    95,     0,     0,
      83,   242,     0,    48,     0,    51,     0,   242,   270,   271,
     118,   120,     0,     0,     0,   212,   183,   184,   182,     0,
     165,   268,   267,   164,     0,     0,     0,     0,   207,   203,
       0,   202,   272,   195,   189,   188,   187,     0,     0,     0,
       0,   108,     0,   110,     0,     0,   106,     0,   242,   234,
      69,     0,    67,    68,     0,   242,   242,   251,     0,   124,
     260,   273,    28,    89,    90,    93,    27,     0,    79,    50,
     274,     0,     0,   225,     0,   226,     0,   186,     0,   174,
       0,   166,     0,   171,   172,   155,   156,   173,   153,   154,
       0,     0,   201,     0,   204,   197,   199,   198,   194,   196,
     280,     0,   170,   169,   176,   177,     0,     0,   116,     0,
     113,     0,     0,    53,     0,    62,    77,    71,    47,     0,
       0,     0,   242,    49,     0,    34,     0,   242,   220,   224,
       0,     0,   266,   211,     0,   209,     0,   210,     0,   277,
     181,   180,   178,   179,   175,   200,     0,   111,   112,   115,
     242,   235,     0,     0,    70,   242,    58,    57,    59,   242,
       0,     0,     0,   129,   137,   140,   138,   215,   216,   139,
     279,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    30,    29,   185,   160,   162,   159,
       0,   157,   158,     0,   206,   208,   205,   190,     0,    74,
      72,    75,    73,     0,     0,     0,     0,   141,   192,   242,
     128,   275,   163,   161,   167,   168,   242,   236,   242,     0,
       0,     0,     0,   191,   130,     0,     0,     0,     0,   218,
       0,   222,     0,   237,   242,     0,   217,     0,   221,     0,
       0,    56,    33,   219,   223,     0,     0,   193
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,   -78,
     -82,  -398,  -100,   155,   -86,   215,  -398,  -398,  -372,  -398,
     -54,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,   173,
    -398,  -398,  -398,  -118,  -398,  -398,   232,  -398,  -398,  -398,
    -398,  -398,   303,  -398,  -398,  -398,   114,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,   -53,  -398,   -88,
    -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,  -334,   130,  -398,  -398,  -398,  -398,  -398,  -398,  -398,
    -398,  -398,  -398,  -398,  -398,     0,  -398,  -398,  -397,  -398,
    -398,  -398,  -398,  -398,  -398,   305,  -398,  -398,  -398,  -398,
    -398,  -398,  -398,  -396,  -383,   306,  -398,  -398,  -137,   -87,
    -120,   -89,  -398,  -398,  -398,  -398,  -398,   263,  -398,   185,
    -398,  -398,  -398,  -177,   199,  -154,  -398,  -398,  -398,  -398,
    -398,  -398,    -6
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     6,     8,     9,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,   301,
     414,    41,   162,   233,    74,    60,    69,   348,   349,   387,
     234,    61,   126,   281,   282,   283,   384,   430,   432,    70,
     347,   111,   299,   115,   103,   161,    75,   229,    76,   230,
      42,    43,   127,   207,   341,   276,   339,   173,    44,    45,
      46,   144,    90,   289,   392,   145,   128,   393,   394,   129,
     187,   318,   188,   421,   443,   189,   253,   190,   444,   191,
     333,   319,   310,   192,   336,   374,   193,   248,   194,   308,
     195,   266,   196,   437,   453,   197,   328,   329,   376,   263,
     322,   366,   368,   364,   198,   130,   396,   397,   458,   131,
     398,   460,   132,   304,   306,   399,   133,   149,   134,   135,
     151,    77,    47,   139,    48,    49,    54,    85,    50,    62,
      97,   156,   223,   254,   240,   158,   355,   268,   225,   401,
     331,    51,    12
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     152,   146,   150,    52,   209,   256,   165,   210,   386,   168,
     116,   117,   159,   433,     5,   163,   153,   163,   241,   164,
     163,   166,   167,   125,   280,   118,   235,   422,   154,    13,
      14,    15,   269,   264,    16,   152,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,   334,   118,   119,
     273,   213,   116,   117,   459,     1,     2,   116,   117,   119,
     120,   246,   325,   326,    58,   470,   335,   118,   461,   208,
     120,   473,   118,   313,   313,     7,   456,   265,   476,   314,
     314,   315,   212,   121,    10,    11,   474,   122,   247,   445,
     277,   119,   471,    72,    73,   235,   119,   123,   390,    59,
     155,    68,   120,   327,   174,   124,   121,   120,   324,   391,
      72,    73,   295,    53,   199,   124,   175,   316,   278,   317,
     317,   251,   200,    10,    11,   121,   313,   417,   279,   122,
     121,   296,   314,   252,   122,   201,   435,   221,   202,   232,
     292,   418,   163,    68,   222,   203,    55,   124,   436,    72,
      73,   302,   124,   380,   124,    71,    91,    92,   344,   204,
     176,   419,   177,   381,    93,    82,   169,    83,   178,    72,
      73,   238,   317,   420,   170,   179,   180,   181,   239,   182,
      56,   183,   205,   206,   439,   423,    94,    95,   257,   152,
     184,   258,   259,    98,   440,   260,   350,   171,    57,   446,
     351,    96,   250,   261,   251,    80,    88,   185,   186,   447,
      84,   172,    89,   475,   112,    86,   252,   113,   114,   427,
      81,   262,   402,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,    63,    64,    65,    66,    67,   218,
     219,    78,    79,    99,   100,   101,   102,   370,   371,   372,
     373,    10,    11,    71,   227,   104,   382,   383,    87,   105,
     428,   138,   106,   152,   395,   150,   107,   108,   109,   110,
     136,   415,   137,   140,   141,   143,   220,   157,   216,   -66,
     211,   224,   160,   245,   217,   226,   242,   231,   214,   236,
     237,   152,   270,   243,   244,   249,   350,   255,   267,   272,
     274,   275,   285,   434,   286,    58,   290,   298,   288,   291,
    -229,   300,   293,   303,   294,   305,   307,   309,   311,   320,
     321,   323,   330,   337,   332,   338,   455,   340,   343,   345,
     353,   346,   352,   354,   356,   358,   359,   363,   357,   365,
     367,   375,   360,   361,   388,   362,   369,   377,   378,   379,
     152,   395,   150,   385,   389,   400,   429,   152,   416,   350,
     424,   425,   426,   431,   452,   448,   427,   441,   442,   449,
     450,   457,   451,   462,   464,   350,   463,   465,   466,   467,
     469,   468,   477,   472,   284,   312,   454,   297,     0,   342,
     142,   438,   228,     0,   147,   148,     0,     0,   271,   287,
       0,     0,   215
};

static const yytype_int16 yycheck[] =
{
      89,    89,    89,     9,   124,   182,   106,   125,    23,   109,
      23,    24,    98,   385,     0,   104,    27,   106,   172,   105,
     109,   107,   108,    77,    23,    38,   163,   361,    39,     5,
       6,     7,   186,    26,    10,   124,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    35,    38,    62,
     204,   137,    23,    24,   450,     3,     4,    23,    24,    62,
      73,    29,    41,    42,    65,   462,    54,    38,   451,   123,
      73,   467,    38,    25,    25,     8,   448,    70,   475,    31,
      31,    33,   136,    96,    99,   100,   469,   100,    56,   423,
     208,    62,   464,   113,   114,   232,    62,   110,    62,   100,
     111,   100,    73,    82,    34,   118,    96,    73,   262,    73,
     113,   114,   230,   108,    29,   118,    46,    69,   109,    71,
      71,    27,    37,    99,   100,    96,    25,    26,   119,   100,
      96,   231,    31,    39,   100,    50,    34,    61,    53,   110,
     226,    40,   231,   100,    68,    60,    99,   118,    46,   113,
     114,   237,   118,   109,   118,    99,    29,    30,   278,    74,
      28,    60,    30,   119,    37,     9,    29,    11,    36,   113,
     114,    61,    71,    72,    37,    43,    44,    45,    68,    47,
      99,    49,    97,    98,   109,   362,    59,    60,    48,   278,
      58,    51,    52,   109,   119,    55,   285,    60,    99,   109,
     286,    74,    25,    63,    27,   108,   111,    75,    76,   119,
      99,    74,   117,   106,   101,   117,    39,   104,   105,   112,
     108,    81,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    19,    20,    21,    22,    23,    23,
      24,    26,    27,   101,   102,   103,   104,    77,    78,    79,
      80,    99,   100,    99,   100,   109,   113,   114,   117,   109,
     380,    99,   109,   352,   352,   352,   109,   109,   109,   104,
     109,   357,   109,   109,   100,    23,    23,   111,    99,   111,
     111,    23,   115,    64,   112,   109,   111,   109,   117,   109,
     109,   380,    66,   111,   111,   111,   385,   111,   111,   111,
     111,   111,   109,   389,   109,    65,   112,   104,   117,   112,
     104,    83,   116,    23,   116,    23,    23,    23,    67,   111,
     111,   111,    23,   111,    29,    23,   446,    23,   110,   112,
     110,   104,   118,   111,   109,   112,   112,    23,   109,    23,
      23,    23,   112,   112,   350,   112,   112,   112,   112,   112,
     439,   439,   439,   109,   109,    23,    23,   446,    57,   448,
     112,   112,   112,    23,    66,   109,   112,   112,    32,   109,
     111,   449,   111,   111,   109,   464,   119,   109,   112,   106,
     106,   112,   112,   465,   211,   255,   439,   232,    -1,   275,
      87,   391,   160,    -1,    89,    89,    -1,    -1,   199,   214,
      -1,    -1,   139
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     4,   121,   122,     0,   123,     8,   124,   125,
      99,   100,   262,     5,     6,     7,    10,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   141,   170,   171,   178,   179,   180,   242,   244,   245,
     248,   261,   262,   108,   246,    99,    99,    99,    65,   100,
     145,   151,   249,   145,   145,   145,   145,   145,   100,   146,
     159,    99,   113,   114,   144,   166,   168,   241,   145,   145,
     108,   108,     9,    11,    99,   247,   117,   117,   111,   117,
     182,    29,    30,    37,    59,    60,    74,   250,   109,   101,
     102,   103,   104,   164,   109,   109,   109,   109,   109,   109,
     104,   161,   101,   104,   105,   163,    23,    24,    38,    62,
      73,    96,   100,   110,   118,   150,   152,   172,   186,   189,
     225,   229,   232,   236,   238,   239,   109,   109,    99,   243,
     109,   100,   172,    23,   181,   185,   189,   225,   235,   237,
     239,   240,   241,    27,    39,   111,   251,   111,   255,   144,
     115,   165,   142,   241,   144,   142,   144,   144,   142,    29,
      37,    60,    74,   177,    34,    46,    28,    30,    36,    43,
      44,    45,    47,    49,    58,    75,    76,   190,   192,   195,
     197,   199,   203,   206,   208,   210,   212,   215,   224,    29,
      37,    50,    53,    60,    74,    97,    98,   173,   150,   240,
     163,   111,   150,   144,   117,   247,    99,   112,    23,    24,
      23,    61,    68,   252,    23,   258,   109,   100,   166,   167,
     169,   109,   110,   143,   150,   238,   109,   109,    61,    68,
     254,   255,   111,   111,   111,    64,    29,    56,   207,   111,
      25,    27,    39,   196,   253,   111,   253,    48,    51,    52,
      55,    63,    81,   219,    26,    70,   211,   111,   257,   255,
      66,   254,   111,   255,   111,   111,   175,   163,   109,   119,
      23,   153,   154,   155,   159,   109,   109,   249,   117,   183,
     112,   112,   144,   116,   116,   163,   142,   143,   104,   162,
      83,   139,   144,    23,   233,    23,   234,    23,   209,    23,
     202,    67,   202,    25,    31,    33,    69,    71,   191,   201,
     111,   111,   220,   111,   255,    41,    42,    82,   216,   217,
      23,   260,    29,   200,    35,    54,   204,   111,    23,   176,
      23,   174,   176,   110,   240,   112,   104,   160,   147,   148,
     241,   144,   118,   110,   111,   256,   109,   109,   112,   112,
     112,   112,   112,    23,   223,    23,   221,    23,   222,   112,
      77,    78,    79,    80,   205,    23,   218,   112,   112,   112,
     109,   119,   113,   114,   156,   109,    23,   149,   262,   109,
      62,    73,   184,   187,   188,   189,   226,   227,   230,   235,
      23,   259,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,   140,   144,    57,    26,    40,    60,
      72,   193,   201,   253,   112,   112,   112,   112,   240,    23,
     157,    23,   158,   148,   144,    34,    46,   213,   215,   109,
     119,   112,    32,   194,   198,   201,   109,   119,   109,   109,
     111,   111,    66,   214,   187,   240,   148,   139,   228,   233,
     231,   234,   111,   119,   109,   109,   112,   106,   112,   106,
     218,   148,   140,   233,   234,   106,   218,   112
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   120,   121,   122,   122,   123,   123,   124,   125,   125,
     126,   126,   127,   127,   128,   128,   128,   128,   128,   128,
     128,   129,   129,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   137,   138,   139,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   141,   142,   142,
     143,   143,   144,   144,   145,   146,   147,   148,   149,   149,
     150,   150,   150,   150,   151,   151,   152,   153,   153,   154,
     155,   156,   156,   156,   157,   158,   159,   160,   161,   162,
     163,   163,   163,   163,   164,   164,   164,   164,   164,   165,
     165,   165,   166,   167,   168,   169,   170,   170,   170,   170,
     170,   170,   171,   172,   172,   173,   173,   173,   173,   173,
     173,   173,   173,   174,   175,   175,   176,   177,   177,   177,
     177,   178,   178,   179,   180,   181,   181,   182,   183,   184,
     184,   185,   185,   185,   186,   186,   186,   187,   187,   187,
     188,   188,   189,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   189,   190,   191,   191,   191,   192,   193,   193,
     193,   193,   193,   194,   195,   196,   196,   197,   198,   199,
     200,   201,   201,   201,   202,   203,   204,   204,   205,   205,
     205,   205,   206,   207,   207,   208,   209,   210,   211,   211,
     212,   213,   214,   214,   215,   216,   216,   217,   217,   217,
     218,   219,   219,   219,   219,   219,   219,   220,   220,   221,
     222,   223,   224,   225,   225,   226,   226,   227,   228,   228,
     229,   230,   231,   231,   232,   233,   234,   235,   235,   236,
     236,   237,   238,   238,   239,   239,   239,   239,   240,   240,
     241,   241,   241,   243,   242,   244,   244,   246,   245,   247,
     247,   248,   249,   249,   249,   249,   249,   249,   250,   251,
     251,   251,   251,   252,   252,   252,   253,   253,   253,   254,
     254,   254,   255,   255,   256,   256,   257,   257,   258,   259,
     260,   261,   262,   262
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     4,     1,     1,     2,     0,     3,     2,     0,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     4,     4,     6,     6,     8,
       8,     2,     2,    12,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     6,     2,     4,
       2,     1,     3,     5,     3,     2,     7,     2,     1,     1,
       1,     1,     4,     1,     1,     1,     1,     1,     1,     1,
       3,     0,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     1,     1,     1,     1,     0,     3,
       3,     0,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     2,     2,     1,     2,     1,     2,     1,
       2,     4,     4,     1,     0,     3,     1,     1,     2,     1,
       2,     1,     1,     3,     6,     0,     1,     2,     4,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     3,     1,     1,     1,     5,     1,     1,
       1,     2,     1,     1,     2,     1,     2,     6,     1,     3,
       1,     1,     1,     1,     1,     4,     1,     1,     1,     1,
       1,     1,     2,     1,     1,     5,     1,     2,     1,     1,
       5,     2,     0,     6,     3,     0,     1,     1,     1,     1,
       1,     2,     1,     1,     2,     4,     4,     0,     3,     1,
       1,     1,     2,     1,     1,     1,     1,     5,     1,     3,
       5,     5,     1,     3,     5,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     5,     7,     9,     2,     2,
       1,     1,     0,     0,     4,     1,     0,     0,     3,     3,
       1,     5,     2,     2,     2,     2,     3,     2,     3,     0,
       3,     1,     1,     0,     1,     1,     0,     1,     1,     0,
       1,     1,     0,     3,     0,     3,     0,     3,     1,     1,
       1,     4,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, state, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, state); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct asm_parser_state *state)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (state);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct asm_parser_state *state)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, state);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, struct asm_parser_state *state)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , state);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, state); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, struct asm_parser_state *state)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (state);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (struct asm_parser_state *state)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc, state);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 290 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->prog->Target != GL_VERTEX_PROGRAM_ARB) {
	      yyerror(& (yylsp[0]), state, "invalid fragment program header");

	   }
	   state->mode = ARB_vertex;
	}
#line 1995 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 298 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->prog->Target != GL_FRAGMENT_PROGRAM_ARB) {
	      yyerror(& (yylsp[0]), state, "invalid vertex program header");
	   }
	   state->mode = ARB_fragment;

	   state->option.TexRect =
	      (state->ctx->Extensions.NV_texture_rectangle != GL_FALSE);
	}
#line 2009 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 314 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   int valid = 0;

	   if (state->mode == ARB_vertex) {
	      valid = _mesa_ARBvp_parse_option(state, (yyvsp[-1].string));
	   } else if (state->mode == ARB_fragment) {
	      valid = _mesa_ARBfp_parse_option(state, (yyvsp[-1].string));
	   }


	   free((yyvsp[-1].string));

	   if (!valid) {
	      const char *const err_str = (state->mode == ARB_vertex)
		 ? "invalid ARB vertex program option"
		 : "invalid ARB fragment program option";

	      yyerror(& (yylsp[-1]), state, err_str);
	      YYERROR;
	   }
	}
#line 2035 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 342 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((yyvsp[-1].inst) != NULL) {
	      if (state->inst_tail == NULL) {
		 state->inst_head = (yyvsp[-1].inst);
	      } else {
		 state->inst_tail->next = (yyvsp[-1].inst);
	      }

	      state->inst_tail = (yyvsp[-1].inst);
	      (yyvsp[-1].inst)->next = NULL;

	      state->prog->NumInstructions++;
	   }
	}
#line 2054 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 360 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = (yyvsp[0].inst);
	   state->prog->NumAluInstructions++;
	}
#line 2063 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 365 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = (yyvsp[0].inst);
	   state->prog->NumTexInstructions++;
	}
#line 2072 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 386 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = asm_instruction_ctor(OPCODE_ARL, & (yyvsp[-2].dst_reg), & (yyvsp[0].src_reg), NULL, NULL);
	}
#line 2080 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 392 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((yyvsp[-3].temp_inst).Opcode == OPCODE_DDY)
	      state->fragment.UsesDFdy = 1;
	   (yyval.inst) = asm_instruction_copy_ctor(& (yyvsp[-3].temp_inst), & (yyvsp[-2].dst_reg), & (yyvsp[0].src_reg), NULL, NULL);
	}
#line 2090 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 400 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = asm_instruction_copy_ctor(& (yyvsp[-3].temp_inst), & (yyvsp[-2].dst_reg), & (yyvsp[0].src_reg), NULL, NULL);
	}
#line 2098 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 406 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = asm_instruction_copy_ctor(& (yyvsp[-5].temp_inst), & (yyvsp[-4].dst_reg), & (yyvsp[-2].src_reg), & (yyvsp[0].src_reg), NULL);
	}
#line 2106 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 413 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = asm_instruction_copy_ctor(& (yyvsp[-5].temp_inst), & (yyvsp[-4].dst_reg), & (yyvsp[-2].src_reg), & (yyvsp[0].src_reg), NULL);
	}
#line 2114 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 420 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = asm_instruction_copy_ctor(& (yyvsp[-7].temp_inst), & (yyvsp[-6].dst_reg), & (yyvsp[-4].src_reg), & (yyvsp[-2].src_reg), & (yyvsp[0].src_reg));
	}
#line 2122 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 426 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = asm_instruction_copy_ctor(& (yyvsp[-7].temp_inst), & (yyvsp[-6].dst_reg), & (yyvsp[-4].src_reg), NULL, NULL);
	   if ((yyval.inst) != NULL) {
	      const GLbitfield tex_mask = (1U << (yyvsp[-2].integer));
	      GLbitfield shadow_tex = 0;
	      GLbitfield target_mask = 0;


	      (yyval.inst)->Base.TexSrcUnit = (yyvsp[-2].integer);

	      if ((yyvsp[0].integer) < 0) {
		 shadow_tex = tex_mask;

		 (yyval.inst)->Base.TexSrcTarget = -(yyvsp[0].integer);
		 (yyval.inst)->Base.TexShadow = 1;
	      } else {
		 (yyval.inst)->Base.TexSrcTarget = (yyvsp[0].integer);
	      }

	      target_mask = (1U << (yyval.inst)->Base.TexSrcTarget);

	      /* If this texture unit was previously accessed and that access
	       * had a different texture target, generate an error.
	       *
	       * If this texture unit was previously accessed and that access
	       * had a different shadow mode, generate an error.
	       */
	      if ((state->prog->TexturesUsed[(yyvsp[-2].integer)] != 0)
		  && ((state->prog->TexturesUsed[(yyvsp[-2].integer)] != target_mask)
		      || ((state->prog->ShadowSamplers & tex_mask)
			  != shadow_tex))) {
		 yyerror(& (yylsp[0]), state,
			 "multiple targets used on one texture image unit");
		 YYERROR;
	      }


	      state->prog->TexturesUsed[(yyvsp[-2].integer)] |= target_mask;
	      state->prog->ShadowSamplers |= shadow_tex;
	   }
	}
#line 2168 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 470 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = asm_instruction_ctor(OPCODE_KIL, NULL, & (yyvsp[0].src_reg), NULL, NULL);
	   state->fragment.UsesKill = 1;
	}
#line 2177 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 475 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = asm_instruction_ctor(OPCODE_KIL_NV, NULL, NULL, NULL, NULL);
	   (yyval.inst)->Base.DstReg.CondMask = (yyvsp[0].dst_reg).CondMask;
	   (yyval.inst)->Base.DstReg.CondSwizzle = (yyvsp[0].dst_reg).CondSwizzle;
	   state->fragment.UsesKill = 1;
	}
#line 2188 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 484 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.inst) = asm_instruction_copy_ctor(& (yyvsp[-11].temp_inst), & (yyvsp[-10].dst_reg), & (yyvsp[-8].src_reg), & (yyvsp[-6].src_reg), & (yyvsp[-4].src_reg));
	   if ((yyval.inst) != NULL) {
	      const GLbitfield tex_mask = (1U << (yyvsp[-2].integer));
	      GLbitfield shadow_tex = 0;
	      GLbitfield target_mask = 0;


	      (yyval.inst)->Base.TexSrcUnit = (yyvsp[-2].integer);

	      if ((yyvsp[0].integer) < 0) {
		 shadow_tex = tex_mask;

		 (yyval.inst)->Base.TexSrcTarget = -(yyvsp[0].integer);
		 (yyval.inst)->Base.TexShadow = 1;
	      } else {
		 (yyval.inst)->Base.TexSrcTarget = (yyvsp[0].integer);
	      }

	      target_mask = (1U << (yyval.inst)->Base.TexSrcTarget);

	      /* If this texture unit was previously accessed and that access
	       * had a different texture target, generate an error.
	       *
	       * If this texture unit was previously accessed and that access
	       * had a different shadow mode, generate an error.
	       */
	      if ((state->prog->TexturesUsed[(yyvsp[-2].integer)] != 0)
		  && ((state->prog->TexturesUsed[(yyvsp[-2].integer)] != target_mask)
		      || ((state->prog->ShadowSamplers & tex_mask)
			  != shadow_tex))) {
		 yyerror(& (yylsp[0]), state,
			 "multiple targets used on one texture image unit");
		 YYERROR;
	      }


	      state->prog->TexturesUsed[(yyvsp[-2].integer)] |= target_mask;
	      state->prog->ShadowSamplers |= shadow_tex;
	   }
	}
#line 2234 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 528 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 2242 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 533 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = TEXTURE_1D_INDEX; }
#line 2248 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 534 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = TEXTURE_2D_INDEX; }
#line 2254 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 535 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = TEXTURE_3D_INDEX; }
#line 2260 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 536 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = TEXTURE_CUBE_INDEX; }
#line 2266 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 537 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = TEXTURE_RECT_INDEX; }
#line 2272 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 538 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = -TEXTURE_1D_INDEX; }
#line 2278 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 539 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = -TEXTURE_2D_INDEX; }
#line 2284 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 540 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = -TEXTURE_RECT_INDEX; }
#line 2290 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 541 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = TEXTURE_1D_ARRAY_INDEX; }
#line 2296 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 542 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = TEXTURE_2D_ARRAY_INDEX; }
#line 2302 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 543 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = -TEXTURE_1D_ARRAY_INDEX; }
#line 2308 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 544 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = -TEXTURE_2D_ARRAY_INDEX; }
#line 2314 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 548 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   /* FIXME: Is this correct?  Should the extenedSwizzle be applied
	    * FIXME: to the existing swizzle?
	    */
	   (yyvsp[-2].src_reg).Base.Swizzle = (yyvsp[0].swiz_mask).swizzle;
	   (yyvsp[-2].src_reg).Base.Negate = (yyvsp[0].swiz_mask).mask;

	   (yyval.inst) = asm_instruction_copy_ctor(& (yyvsp[-5].temp_inst), & (yyvsp[-4].dst_reg), & (yyvsp[-2].src_reg), NULL, NULL);
	}
#line 2328 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 560 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.src_reg) = (yyvsp[0].src_reg);

	   if ((yyvsp[-1].negate)) {
	      (yyval.src_reg).Base.Negate = ~(yyval.src_reg).Base.Negate;
	   }
	}
#line 2340 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 568 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.src_reg) = (yyvsp[-1].src_reg);

	   if (!state->option.NV_fragment) {
	      yyerror(& (yylsp[-2]), state, "unexpected character '|'");
	      YYERROR;
	   }

	   if ((yyvsp[-3].negate)) {
	      (yyval.src_reg).Base.Negate = ~(yyval.src_reg).Base.Negate;
	   }

	   (yyval.src_reg).Base.Abs = 1;
	}
#line 2359 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 585 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.src_reg) = (yyvsp[-1].src_reg);

	   (yyval.src_reg).Base.Swizzle = _mesa_combine_swizzles((yyval.src_reg).Base.Swizzle,
						    (yyvsp[0].swiz_mask).swizzle);
	}
#line 2370 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 592 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   struct asm_symbol temp_sym;

	   if (!state->option.NV_fragment) {
	      yyerror(& (yylsp[0]), state, "expected scalar suffix");
	      YYERROR;
	   }

	   memset(& temp_sym, 0, sizeof(temp_sym));
	   temp_sym.param_binding_begin = ~0;
	   initialize_symbol_from_const(state->prog, & temp_sym, & (yyvsp[0].vector), GL_TRUE);

	   set_src_reg_swz(& (yyval.src_reg), PROGRAM_CONSTANT,
                           temp_sym.param_binding_begin,
                           temp_sym.param_binding_swizzle);
	}
#line 2391 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 611 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.src_reg) = (yyvsp[-1].src_reg);

	   if ((yyvsp[-2].negate)) {
	      (yyval.src_reg).Base.Negate = ~(yyval.src_reg).Base.Negate;
	   }

	   (yyval.src_reg).Base.Swizzle = _mesa_combine_swizzles((yyval.src_reg).Base.Swizzle,
						    (yyvsp[0].swiz_mask).swizzle);
	}
#line 2406 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 622 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.src_reg) = (yyvsp[-2].src_reg);

	   if (!state->option.NV_fragment) {
	      yyerror(& (yylsp[-3]), state, "unexpected character '|'");
	      YYERROR;
	   }

	   if ((yyvsp[-4].negate)) {
	      (yyval.src_reg).Base.Negate = ~(yyval.src_reg).Base.Negate;
	   }

	   (yyval.src_reg).Base.Abs = 1;
	   (yyval.src_reg).Base.Swizzle = _mesa_combine_swizzles((yyval.src_reg).Base.Swizzle,
						    (yyvsp[-1].swiz_mask).swizzle);
	}
#line 2427 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 642 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.dst_reg) = (yyvsp[-2].dst_reg);
	   (yyval.dst_reg).WriteMask = (yyvsp[-1].swiz_mask).mask;
	   (yyval.dst_reg).CondMask = (yyvsp[0].dst_reg).CondMask;
	   (yyval.dst_reg).CondSwizzle = (yyvsp[0].dst_reg).CondSwizzle;

	   if ((yyval.dst_reg).File == PROGRAM_OUTPUT) {
	      /* Technically speaking, this should check that it is in
	       * vertex program mode.  However, PositionInvariant can never be
	       * set in fragment program mode, so it is somewhat irrelevant.
	       */
	      if (state->option.PositionInvariant
	       && ((yyval.dst_reg).Index == VARYING_SLOT_POS)) {
		 yyerror(& (yylsp[-2]), state, "position-invariant programs cannot "
			 "write position");
		 YYERROR;
	      }

	      state->prog->OutputsWritten |= BITFIELD64_BIT((yyval.dst_reg).Index);
	   }
	}
#line 2453 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 666 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   set_dst_reg(& (yyval.dst_reg), PROGRAM_ADDRESS, 0);
	   (yyval.dst_reg).WriteMask = (yyvsp[0].swiz_mask).mask;
	}
#line 2462 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 673 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   const unsigned xyzw_valid =
	      ((yyvsp[-6].ext_swizzle).xyzw_valid << 0)
	      | ((yyvsp[-4].ext_swizzle).xyzw_valid << 1)
	      | ((yyvsp[-2].ext_swizzle).xyzw_valid << 2)
	      | ((yyvsp[0].ext_swizzle).xyzw_valid << 3);
	   const unsigned rgba_valid =
	      ((yyvsp[-6].ext_swizzle).rgba_valid << 0)
	      | ((yyvsp[-4].ext_swizzle).rgba_valid << 1)
	      | ((yyvsp[-2].ext_swizzle).rgba_valid << 2)
	      | ((yyvsp[0].ext_swizzle).rgba_valid << 3);

	   /* All of the swizzle components have to be valid in either RGBA
	    * or XYZW.  Note that 0 and 1 are valid in both, so both masks
	    * can have some bits set.
	    *
	    * We somewhat deviate from the spec here.  It would be really hard
	    * to figure out which component is the error, and there probably
	    * isn't a lot of benefit.
	    */
	   if ((rgba_valid != 0x0f) && (xyzw_valid != 0x0f)) {
	      yyerror(& (yylsp[-6]), state, "cannot combine RGBA and XYZW swizzle "
		      "components");
	      YYERROR;
	   }

	   (yyval.swiz_mask).swizzle = MAKE_SWIZZLE4((yyvsp[-6].ext_swizzle).swz, (yyvsp[-4].ext_swizzle).swz, (yyvsp[-2].ext_swizzle).swz, (yyvsp[0].ext_swizzle).swz);
	   (yyval.swiz_mask).mask = ((yyvsp[-6].ext_swizzle).negate) | ((yyvsp[-4].ext_swizzle).negate << 1) | ((yyvsp[-2].ext_swizzle).negate << 2)
	      | ((yyvsp[0].ext_swizzle).negate << 3);
	}
#line 2497 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 706 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.ext_swizzle) = (yyvsp[0].ext_swizzle);
	   (yyval.ext_swizzle).negate = ((yyvsp[-1].negate)) ? 1 : 0;
	}
#line 2506 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 713 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (((yyvsp[0].integer) != 0) && ((yyvsp[0].integer) != 1)) {
	      yyerror(& (yylsp[0]), state, "invalid extended swizzle selector");
	      YYERROR;
	   }

	   (yyval.ext_swizzle).swz = ((yyvsp[0].integer) == 0) ? SWIZZLE_ZERO : SWIZZLE_ONE;
           (yyval.ext_swizzle).negate = 0;

	   /* 0 and 1 are valid for both RGBA swizzle names and XYZW
	    * swizzle names.
	    */
	   (yyval.ext_swizzle).xyzw_valid = 1;
	   (yyval.ext_swizzle).rgba_valid = 1;
	}
#line 2526 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 729 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   char s;

	   if (strlen((yyvsp[0].string)) > 1) {
	      yyerror(& (yylsp[0]), state, "invalid extended swizzle selector");
	      YYERROR;
	   }

	   s = (yyvsp[0].string)[0];
	   free((yyvsp[0].string));

           (yyval.ext_swizzle).rgba_valid = 0;
           (yyval.ext_swizzle).xyzw_valid = 0;
           (yyval.ext_swizzle).negate = 0;

	   switch (s) {
	   case 'x':
	      (yyval.ext_swizzle).swz = SWIZZLE_X;
	      (yyval.ext_swizzle).xyzw_valid = 1;
	      break;
	   case 'y':
	      (yyval.ext_swizzle).swz = SWIZZLE_Y;
	      (yyval.ext_swizzle).xyzw_valid = 1;
	      break;
	   case 'z':
	      (yyval.ext_swizzle).swz = SWIZZLE_Z;
	      (yyval.ext_swizzle).xyzw_valid = 1;
	      break;
	   case 'w':
	      (yyval.ext_swizzle).swz = SWIZZLE_W;
	      (yyval.ext_swizzle).xyzw_valid = 1;
	      break;

	   case 'r':
	      (yyval.ext_swizzle).swz = SWIZZLE_X;
	      (yyval.ext_swizzle).rgba_valid = 1;
	      break;
	   case 'g':
	      (yyval.ext_swizzle).swz = SWIZZLE_Y;
	      (yyval.ext_swizzle).rgba_valid = 1;
	      break;
	   case 'b':
	      (yyval.ext_swizzle).swz = SWIZZLE_Z;
	      (yyval.ext_swizzle).rgba_valid = 1;
	      break;
	   case 'a':
	      (yyval.ext_swizzle).swz = SWIZZLE_W;
	      (yyval.ext_swizzle).rgba_valid = 1;
	      break;

	   default:
	      yyerror(& (yylsp[0]), state, "invalid extended swizzle selector");
	      YYERROR;
	      break;
	   }
	}
#line 2587 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 788 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   struct asm_symbol *const s = (struct asm_symbol *)
	      _mesa_symbol_table_find_symbol(state->st, 0, (yyvsp[0].string));

	   free((yyvsp[0].string));

	   if (s == NULL) {
	      yyerror(& (yylsp[0]), state, "invalid operand variable");
	      YYERROR;
	   } else if ((s->type != at_param) && (s->type != at_temp)
		      && (s->type != at_attrib)) {
	      yyerror(& (yylsp[0]), state, "invalid operand variable");
	      YYERROR;
	   } else if ((s->type == at_param) && s->param_is_array) {
	      yyerror(& (yylsp[0]), state, "non-array access to array PARAM");
	      YYERROR;
	   }

	   init_src_reg(& (yyval.src_reg));
	   switch (s->type) {
	   case at_temp:
	      set_src_reg(& (yyval.src_reg), PROGRAM_TEMPORARY, s->temp_binding);
	      break;
	   case at_param:
              set_src_reg_swz(& (yyval.src_reg), s->param_binding_type,
                              s->param_binding_begin,
                              s->param_binding_swizzle);
	      break;
	   case at_attrib:
	      set_src_reg(& (yyval.src_reg), PROGRAM_INPUT, s->attrib_binding);
	      state->prog->InputsRead |= BITFIELD64_BIT((yyval.src_reg).Base.Index);

	      if (!validate_inputs(& (yylsp[0]), state)) {
		 YYERROR;
	      }
	      break;

	   default:
	      YYERROR;
	      break;
	   }
	}
#line 2634 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 831 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   set_src_reg(& (yyval.src_reg), PROGRAM_INPUT, (yyvsp[0].attrib));
	   state->prog->InputsRead |= BITFIELD64_BIT((yyval.src_reg).Base.Index);

	   if (!validate_inputs(& (yylsp[0]), state)) {
	      YYERROR;
	   }
	}
#line 2647 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 840 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (! (yyvsp[-1].src_reg).Base.RelAddr
	       && ((unsigned) (yyvsp[-1].src_reg).Base.Index >= (yyvsp[-3].sym)->param_binding_length)) {
	      yyerror(& (yylsp[-1]), state, "out of bounds array access");
	      YYERROR;
	   }

	   init_src_reg(& (yyval.src_reg));
	   (yyval.src_reg).Base.File = (yyvsp[-3].sym)->param_binding_type;

	   if ((yyvsp[-1].src_reg).Base.RelAddr) {
              state->prog->IndirectRegisterFiles |= (1 << (yyval.src_reg).Base.File);
	      (yyvsp[-3].sym)->param_accessed_indirectly = 1;

	      (yyval.src_reg).Base.RelAddr = 1;
	      (yyval.src_reg).Base.Index = (yyvsp[-1].src_reg).Base.Index;
	      (yyval.src_reg).Symbol = (yyvsp[-3].sym);
	   } else {
	      (yyval.src_reg).Base.Index = (yyvsp[-3].sym)->param_binding_begin + (yyvsp[-1].src_reg).Base.Index;
	   }
	}
#line 2673 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 862 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
           gl_register_file file = ((yyvsp[0].temp_sym).name != NULL) 
	      ? (yyvsp[0].temp_sym).param_binding_type
	      : PROGRAM_CONSTANT;
           set_src_reg_swz(& (yyval.src_reg), file, (yyvsp[0].temp_sym).param_binding_begin,
                           (yyvsp[0].temp_sym).param_binding_swizzle);
	}
#line 2685 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 872 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   set_dst_reg(& (yyval.dst_reg), PROGRAM_OUTPUT, (yyvsp[0].result));
	}
#line 2693 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 876 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   struct asm_symbol *const s = (struct asm_symbol *)
	      _mesa_symbol_table_find_symbol(state->st, 0, (yyvsp[0].string));

	   free((yyvsp[0].string));

	   if (s == NULL) {
	      yyerror(& (yylsp[0]), state, "invalid operand variable");
	      YYERROR;
	   } else if ((s->type != at_output) && (s->type != at_temp)) {
	      yyerror(& (yylsp[0]), state, "invalid operand variable");
	      YYERROR;
	   }

	   switch (s->type) {
	   case at_temp:
	      set_dst_reg(& (yyval.dst_reg), PROGRAM_TEMPORARY, s->temp_binding);
	      break;
	   case at_output:
	      set_dst_reg(& (yyval.dst_reg), PROGRAM_OUTPUT, s->output_binding);
	      break;
	   default:
	      set_dst_reg(& (yyval.dst_reg), s->param_binding_type, s->param_binding_begin);
	      break;
	   }
	}
#line 2724 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 905 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   struct asm_symbol *const s = (struct asm_symbol *)
	      _mesa_symbol_table_find_symbol(state->st, 0, (yyvsp[0].string));

	   free((yyvsp[0].string));

	   if (s == NULL) {
	      yyerror(& (yylsp[0]), state, "invalid operand variable");
	      YYERROR;
	   } else if ((s->type != at_param) || !s->param_is_array) {
	      yyerror(& (yylsp[0]), state, "array access to non-PARAM variable");
	      YYERROR;
	   } else {
	      (yyval.sym) = s;
	   }
	}
#line 2745 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 926 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   init_src_reg(& (yyval.src_reg));
	   (yyval.src_reg).Base.Index = (yyvsp[0].integer);
	}
#line 2754 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 933 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   /* FINISHME: Add support for multiple address registers.
	    */
	   /* FINISHME: Add support for 4-component address registers.
	    */
	   init_src_reg(& (yyval.src_reg));
	   (yyval.src_reg).Base.RelAddr = 1;
	   (yyval.src_reg).Base.Index = (yyvsp[0].integer);
	}
#line 2768 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 944 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = 0; }
#line 2774 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 945 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = (yyvsp[0].integer); }
#line 2780 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 946 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = -(yyvsp[0].integer); }
#line 2786 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 950 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (((yyvsp[0].integer) < 0) || ((yyvsp[0].integer) > (state->limits->MaxAddressOffset - 1))) {
              char s[100];
              _mesa_snprintf(s, sizeof(s),
                             "relative address offset too large (%d)", (yyvsp[0].integer));
	      yyerror(& (yylsp[0]), state, s);
	      YYERROR;
	   } else {
	      (yyval.integer) = (yyvsp[0].integer);
	   }
	}
#line 2802 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 964 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (((yyvsp[0].integer) < 0) || ((yyvsp[0].integer) > state->limits->MaxAddressOffset)) {
              char s[100];
              _mesa_snprintf(s, sizeof(s),
                             "relative address offset too large (%d)", (yyvsp[0].integer));
	      yyerror(& (yylsp[0]), state, s);
	      YYERROR;
	   } else {
	      (yyval.integer) = (yyvsp[0].integer);
	   }
	}
#line 2818 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 978 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   struct asm_symbol *const s = (struct asm_symbol *)
	      _mesa_symbol_table_find_symbol(state->st, 0, (yyvsp[0].string));

	   free((yyvsp[0].string));

	   if (s == NULL) {
	      yyerror(& (yylsp[0]), state, "invalid array member");
	      YYERROR;
	   } else if (s->type != at_address) {
	      yyerror(& (yylsp[0]), state,
		      "invalid variable for indexed array access");
	      YYERROR;
	   } else {
	      (yyval.sym) = s;
	   }
	}
#line 2840 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 998 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((yyvsp[0].swiz_mask).mask != WRITEMASK_X) {
	      yyerror(& (yylsp[0]), state, "invalid address component selector");
	      YYERROR;
	   } else {
	      (yyval.swiz_mask) = (yyvsp[0].swiz_mask);
	   }
	}
#line 2853 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 1009 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((yyvsp[0].swiz_mask).mask != WRITEMASK_X) {
	      yyerror(& (yylsp[0]), state,
		      "address register write mask must be \".x\"");
	      YYERROR;
	   } else {
	      (yyval.swiz_mask) = (yyvsp[0].swiz_mask);
	   }
	}
#line 2867 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 1025 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.swiz_mask).swizzle = SWIZZLE_NOOP; (yyval.swiz_mask).mask = WRITEMASK_XYZW; }
#line 2873 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 1029 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.swiz_mask).swizzle = SWIZZLE_NOOP; (yyval.swiz_mask).mask = WRITEMASK_XYZW; }
#line 2879 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 1033 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.dst_reg) = (yyvsp[-1].dst_reg);
	}
#line 2887 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 1037 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.dst_reg) = (yyvsp[-1].dst_reg);
	}
#line 2895 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 1041 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.dst_reg).CondMask = COND_TR;
	   (yyval.dst_reg).CondSwizzle = SWIZZLE_NOOP;
	}
#line 2904 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 1048 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.dst_reg) = (yyvsp[-1].dst_reg);
	   (yyval.dst_reg).CondSwizzle = (yyvsp[0].swiz_mask).swizzle;
	}
#line 2913 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 1055 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.dst_reg) = (yyvsp[-1].dst_reg);
	   (yyval.dst_reg).CondSwizzle = (yyvsp[0].swiz_mask).swizzle;
	}
#line 2922 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 1062 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   const int cond = _mesa_parse_cc((yyvsp[0].string));
	   if ((cond == 0) || ((yyvsp[0].string)[2] != '\0')) {
	      char *const err_str =
		 make_error_string("invalid condition code \"%s\"", (yyvsp[0].string));

	      yyerror(& (yylsp[0]), state, (err_str != NULL)
		      ? err_str : "invalid condition code");

	      if (err_str != NULL) {
		 free(err_str);
	      }

	      YYERROR;
	   }

	   (yyval.dst_reg).CondMask = cond;
	   (yyval.dst_reg).CondSwizzle = SWIZZLE_NOOP;
	}
#line 2946 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 1084 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   const int cond = _mesa_parse_cc((yyvsp[0].string));
	   if ((cond == 0) || ((yyvsp[0].string)[2] != '\0')) {
	      char *const err_str =
		 make_error_string("invalid condition code \"%s\"", (yyvsp[0].string));

	      yyerror(& (yylsp[0]), state, (err_str != NULL)
		      ? err_str : "invalid condition code");

	      if (err_str != NULL) {
		 free(err_str);
	      }

	      YYERROR;
	   }

	   (yyval.dst_reg).CondMask = cond;
	   (yyval.dst_reg).CondSwizzle = SWIZZLE_NOOP;
	}
#line 2970 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1114 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   struct asm_symbol *const s =
	      declare_variable(state, (yyvsp[-2].string), at_attrib, & (yylsp[-2]));

	   if (s == NULL) {
	      free((yyvsp[-2].string));
	      YYERROR;
	   } else {
	      s->attrib_binding = (yyvsp[0].attrib);
	      state->InputsBound |= BITFIELD64_BIT(s->attrib_binding);

	      if (!validate_inputs(& (yylsp[0]), state)) {
		 YYERROR;
	      }
	   }
	}
#line 2991 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 1133 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = (yyvsp[0].attrib);
	}
#line 2999 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1137 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = (yyvsp[0].attrib);
	}
#line 3007 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1143 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VERT_ATTRIB_POS;
	}
#line 3015 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1147 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VERT_ATTRIB_WEIGHT;
	}
#line 3023 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1151 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VERT_ATTRIB_NORMAL;
	}
#line 3031 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1155 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VERT_ATTRIB_COLOR0 + (yyvsp[0].integer);
	}
#line 3039 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1159 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VERT_ATTRIB_FOG;
	}
#line 3047 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1163 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VERT_ATTRIB_TEX0 + (yyvsp[0].integer);
	}
#line 3055 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 1167 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   yyerror(& (yylsp[-3]), state, "GL_ARB_matrix_palette not supported");
	   YYERROR;
	}
#line 3064 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1172 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VERT_ATTRIB_GENERIC0 + (yyvsp[-1].integer);
	}
#line 3072 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1178 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((unsigned) (yyvsp[0].integer) >= state->limits->MaxAttribs) {
	      yyerror(& (yylsp[0]), state, "invalid vertex attribute reference");
	      YYERROR;
	   }

	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3085 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1192 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VARYING_SLOT_POS;
	}
#line 3093 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1196 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VARYING_SLOT_COL0 + (yyvsp[0].integer);
	}
#line 3101 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1200 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VARYING_SLOT_FOGC;
	}
#line 3109 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1204 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.attrib) = VARYING_SLOT_TEX0 + (yyvsp[0].integer);
	}
#line 3117 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1212 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   struct asm_symbol *const s =
	      declare_variable(state, (yyvsp[-1].string), at_param, & (yylsp[-1]));

	   if (s == NULL) {
	      free((yyvsp[-1].string));
	      YYERROR;
	   } else {
	      s->param_binding_type = (yyvsp[0].temp_sym).param_binding_type;
	      s->param_binding_begin = (yyvsp[0].temp_sym).param_binding_begin;
	      s->param_binding_length = (yyvsp[0].temp_sym).param_binding_length;
              s->param_binding_swizzle = (yyvsp[0].temp_sym).param_binding_swizzle;
	      s->param_is_array = 0;
	   }
	}
#line 3137 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 1230 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (((yyvsp[-2].integer) != 0) && ((unsigned) (yyvsp[-2].integer) != (yyvsp[0].temp_sym).param_binding_length)) {
	      free((yyvsp[-4].string));
	      yyerror(& (yylsp[-2]), state, 
		      "parameter array size and number of bindings must match");
	      YYERROR;
	   } else {
	      struct asm_symbol *const s =
		 declare_variable(state, (yyvsp[-4].string), (yyvsp[0].temp_sym).type, & (yylsp[-4]));

	      if (s == NULL) {
		 free((yyvsp[-4].string));
		 YYERROR;
	      } else {
		 s->param_binding_type = (yyvsp[0].temp_sym).param_binding_type;
		 s->param_binding_begin = (yyvsp[0].temp_sym).param_binding_begin;
		 s->param_binding_length = (yyvsp[0].temp_sym).param_binding_length;
                 s->param_binding_swizzle = SWIZZLE_XYZW;
		 s->param_is_array = 1;
	      }
	   }
	}
#line 3164 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1255 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = 0;
	}
#line 3172 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1259 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (((yyvsp[0].integer) < 1) || ((unsigned) (yyvsp[0].integer) > state->limits->MaxParameters)) {
              char msg[100];
              _mesa_snprintf(msg, sizeof(msg),
                             "invalid parameter array size (size=%d max=%u)",
                             (yyvsp[0].integer), state->limits->MaxParameters);
	      yyerror(& (yylsp[0]), state, msg);
	      YYERROR;
	   } else {
	      (yyval.integer) = (yyvsp[0].integer);
	   }
	}
#line 3189 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1274 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.temp_sym) = (yyvsp[0].temp_sym);
	}
#line 3197 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1280 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.temp_sym) = (yyvsp[-1].temp_sym);
	}
#line 3205 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 1287 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyvsp[-2].temp_sym).param_binding_length += (yyvsp[0].temp_sym).param_binding_length;
	   (yyval.temp_sym) = (yyvsp[-2].temp_sym);
	}
#line 3214 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1294 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset(& (yyval.temp_sym), 0, sizeof((yyval.temp_sym)));
	   (yyval.temp_sym).param_binding_begin = ~0;
	   initialize_symbol_from_state(state->prog, & (yyval.temp_sym), (yyvsp[0].state));
	}
#line 3224 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1300 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset(& (yyval.temp_sym), 0, sizeof((yyval.temp_sym)));
	   (yyval.temp_sym).param_binding_begin = ~0;
	   initialize_symbol_from_param(state->prog, & (yyval.temp_sym), (yyvsp[0].state));
	}
#line 3234 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1306 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset(& (yyval.temp_sym), 0, sizeof((yyval.temp_sym)));
	   (yyval.temp_sym).param_binding_begin = ~0;
	   initialize_symbol_from_const(state->prog, & (yyval.temp_sym), & (yyvsp[0].vector), GL_TRUE);
	}
#line 3244 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1314 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset(& (yyval.temp_sym), 0, sizeof((yyval.temp_sym)));
	   (yyval.temp_sym).param_binding_begin = ~0;
	   initialize_symbol_from_state(state->prog, & (yyval.temp_sym), (yyvsp[0].state));
	}
#line 3254 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1320 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset(& (yyval.temp_sym), 0, sizeof((yyval.temp_sym)));
	   (yyval.temp_sym).param_binding_begin = ~0;
	   initialize_symbol_from_param(state->prog, & (yyval.temp_sym), (yyvsp[0].state));
	}
#line 3264 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1326 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset(& (yyval.temp_sym), 0, sizeof((yyval.temp_sym)));
	   (yyval.temp_sym).param_binding_begin = ~0;
	   initialize_symbol_from_const(state->prog, & (yyval.temp_sym), & (yyvsp[0].vector), GL_TRUE);
	}
#line 3274 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1334 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset(& (yyval.temp_sym), 0, sizeof((yyval.temp_sym)));
	   (yyval.temp_sym).param_binding_begin = ~0;
	   initialize_symbol_from_state(state->prog, & (yyval.temp_sym), (yyvsp[0].state));
	}
#line 3284 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1340 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset(& (yyval.temp_sym), 0, sizeof((yyval.temp_sym)));
	   (yyval.temp_sym).param_binding_begin = ~0;
	   initialize_symbol_from_param(state->prog, & (yyval.temp_sym), (yyvsp[0].state));
	}
#line 3294 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1346 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset(& (yyval.temp_sym), 0, sizeof((yyval.temp_sym)));
	   (yyval.temp_sym).param_binding_begin = ~0;
	   initialize_symbol_from_const(state->prog, & (yyval.temp_sym), & (yyvsp[0].vector), GL_FALSE);
	}
#line 3304 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1353 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3310 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1354 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3316 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1357 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3322 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1358 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3328 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1359 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3334 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1360 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3340 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1361 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3346 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1362 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3352 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1363 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3358 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1364 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3364 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1365 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3370 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1366 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3376 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1367 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { memcpy((yyval.state), (yyvsp[0].state), sizeof((yyval.state))); }
#line 3382 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1371 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = STATE_MATERIAL;
	   (yyval.state)[1] = (yyvsp[-1].integer);
	   (yyval.state)[2] = (yyvsp[0].integer);
	}
#line 3393 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1380 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3401 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1384 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_EMISSION;
	}
#line 3409 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1388 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_SHININESS;
	}
#line 3417 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1394 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = STATE_LIGHT;
	   (yyval.state)[1] = (yyvsp[-2].integer);
	   (yyval.state)[2] = (yyvsp[0].integer);
	}
#line 3428 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1403 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3436 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1407 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_POSITION;
	}
#line 3444 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1411 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (!state->ctx->Extensions.EXT_point_parameters) {
	      yyerror(& (yylsp[0]), state, "GL_ARB_point_parameters not supported");
	      YYERROR;
	   }

	   (yyval.integer) = STATE_ATTENUATION;
	}
#line 3457 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1420 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3465 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1424 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_HALF_VECTOR;
	}
#line 3473 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1430 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_SPOT_DIRECTION;
	}
#line 3481 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1436 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = (yyvsp[0].state)[0];
	   (yyval.state)[1] = (yyvsp[0].state)[1];
	}
#line 3490 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1443 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = STATE_LIGHTMODEL_AMBIENT;
	}
#line 3499 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1448 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = STATE_LIGHTMODEL_SCENECOLOR;
	   (yyval.state)[1] = (yyvsp[-1].integer);
	}
#line 3509 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1456 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = STATE_LIGHTPROD;
	   (yyval.state)[1] = (yyvsp[-3].integer);
	   (yyval.state)[2] = (yyvsp[-1].integer);
	   (yyval.state)[3] = (yyvsp[0].integer);
	}
#line 3521 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1468 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = (yyvsp[0].integer);
	   (yyval.state)[1] = (yyvsp[-1].integer);
	}
#line 3531 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1476 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_TEXENV_COLOR;
	}
#line 3539 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1482 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_AMBIENT;
	}
#line 3547 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1486 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_DIFFUSE;
	}
#line 3555 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1490 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_SPECULAR;
	}
#line 3563 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1496 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((unsigned) (yyvsp[0].integer) >= state->MaxLights) {
	      yyerror(& (yylsp[0]), state, "invalid light selector");
	      YYERROR;
	   }

	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3576 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1507 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = STATE_TEXGEN;
	   (yyval.state)[1] = (yyvsp[-2].integer);
	   (yyval.state)[2] = (yyvsp[-1].integer) + (yyvsp[0].integer);
	}
#line 3587 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1516 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_TEXGEN_EYE_S;
	}
#line 3595 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1520 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_TEXGEN_OBJECT_S;
	}
#line 3603 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1525 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_TEXGEN_EYE_S - STATE_TEXGEN_EYE_S;
	}
#line 3611 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1529 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_TEXGEN_EYE_T - STATE_TEXGEN_EYE_S;
	}
#line 3619 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1533 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_TEXGEN_EYE_R - STATE_TEXGEN_EYE_S;
	}
#line 3627 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1537 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_TEXGEN_EYE_Q - STATE_TEXGEN_EYE_S;
	}
#line 3635 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1543 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = (yyvsp[0].integer);
	}
#line 3644 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1550 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_FOG_COLOR;
	}
#line 3652 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1554 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_FOG_PARAMS;
	}
#line 3660 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1560 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = STATE_CLIPPLANE;
	   (yyval.state)[1] = (yyvsp[-2].integer);
	}
#line 3670 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1568 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((unsigned) (yyvsp[0].integer) >= state->MaxClipPlanes) {
	      yyerror(& (yylsp[0]), state, "invalid clip plane selector");
	      YYERROR;
	   }

	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3683 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1579 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = (yyvsp[0].integer);
	}
#line 3692 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1586 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_POINT_SIZE;
	}
#line 3700 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 1590 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_POINT_ATTENUATION;
	}
#line 3708 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 1596 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = (yyvsp[-4].state)[0];
	   (yyval.state)[1] = (yyvsp[-4].state)[1];
	   (yyval.state)[2] = (yyvsp[-1].integer);
	   (yyval.state)[3] = (yyvsp[-1].integer);
	   (yyval.state)[4] = (yyvsp[-4].state)[2];
	}
#line 3720 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 1606 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = (yyvsp[-1].state)[0];
	   (yyval.state)[1] = (yyvsp[-1].state)[1];
	   (yyval.state)[2] = (yyvsp[0].state)[2];
	   (yyval.state)[3] = (yyvsp[0].state)[3];
	   (yyval.state)[4] = (yyvsp[-1].state)[2];
	}
#line 3732 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 1616 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[2] = 0;
	   (yyval.state)[3] = 3;
	}
#line 3741 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 1621 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   /* It seems logical that the matrix row range specifier would have
	    * to specify a range or more than one row (i.e., $5 > $3).
	    * However, the ARB_vertex_program spec says "a program will fail
	    * to load if <a> is greater than <b>."  This means that $3 == $5
	    * is valid.
	    */
	   if ((yyvsp[-3].integer) > (yyvsp[-1].integer)) {
	      yyerror(& (yylsp[-3]), state, "invalid matrix row range");
	      YYERROR;
	   }

	   (yyval.state)[2] = (yyvsp[-3].integer);
	   (yyval.state)[3] = (yyvsp[-1].integer);
	}
#line 3761 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1639 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = (yyvsp[-1].state)[0];
	   (yyval.state)[1] = (yyvsp[-1].state)[1];
	   (yyval.state)[2] = (yyvsp[0].integer);
	}
#line 3771 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1647 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = 0;
	}
#line 3779 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 1651 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3787 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 1657 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_MATRIX_INVERSE;
	}
#line 3795 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 1661 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_MATRIX_TRANSPOSE;
	}
#line 3803 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 1665 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = STATE_MATRIX_INVTRANS;
	}
#line 3811 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 1671 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((yyvsp[0].integer) > 3) {
	      yyerror(& (yylsp[0]), state, "invalid matrix row reference");
	      YYERROR;
	   }

	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3824 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 1682 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = STATE_MODELVIEW_MATRIX;
	   (yyval.state)[1] = (yyvsp[0].integer);
	}
#line 3833 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 1687 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = STATE_PROJECTION_MATRIX;
	   (yyval.state)[1] = 0;
	}
#line 3842 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 1692 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = STATE_MVP_MATRIX;
	   (yyval.state)[1] = 0;
	}
#line 3851 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 1697 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = STATE_TEXTURE_MATRIX;
	   (yyval.state)[1] = (yyvsp[0].integer);
	}
#line 3860 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 1702 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   yyerror(& (yylsp[-3]), state, "GL_ARB_matrix_palette not supported");
	   YYERROR;
	}
#line 3869 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 1707 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = STATE_PROGRAM_MATRIX;
	   (yyval.state)[1] = (yyvsp[-1].integer);
	}
#line 3878 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 1714 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = 0;
	}
#line 3886 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 1718 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = (yyvsp[-1].integer);
	}
#line 3894 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 1723 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   /* Since GL_ARB_vertex_blend isn't supported, only modelview matrix
	    * zero is valid.
	    */
	   if ((yyvsp[0].integer) != 0) {
	      yyerror(& (yylsp[0]), state, "invalid modelview matrix index");
	      YYERROR;
	   }

	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3910 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 1736 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   /* Since GL_ARB_matrix_palette isn't supported, just let any value
	    * through here.  The error will be generated later.
	    */
	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3921 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 1744 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((unsigned) (yyvsp[0].integer) >= state->MaxProgramMatrices) {
	      yyerror(& (yylsp[0]), state, "invalid program matrix selector");
	      YYERROR;
	   }

	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 3934 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 1755 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = STATE_DEPTH_RANGE;
	}
#line 3943 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 1767 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = state->state_param_enum;
	   (yyval.state)[1] = STATE_ENV;
	   (yyval.state)[2] = (yyvsp[-1].state)[0];
	   (yyval.state)[3] = (yyvsp[-1].state)[1];
	}
#line 3955 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 1777 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = (yyvsp[0].integer);
	   (yyval.state)[1] = (yyvsp[0].integer);
	}
#line 3964 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 1782 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = (yyvsp[-2].integer);
	   (yyval.state)[1] = (yyvsp[0].integer);
	}
#line 3973 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 1789 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = state->state_param_enum;
	   (yyval.state)[1] = STATE_ENV;
	   (yyval.state)[2] = (yyvsp[-1].integer);
	   (yyval.state)[3] = (yyvsp[-1].integer);
	}
#line 3985 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 1799 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = state->state_param_enum;
	   (yyval.state)[1] = STATE_LOCAL;
	   (yyval.state)[2] = (yyvsp[-1].state)[0];
	   (yyval.state)[3] = (yyvsp[-1].state)[1];
	}
#line 3997 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 1808 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = (yyvsp[0].integer);
	   (yyval.state)[1] = (yyvsp[0].integer);
	}
#line 4006 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 1813 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.state)[0] = (yyvsp[-2].integer);
	   (yyval.state)[1] = (yyvsp[0].integer);
	}
#line 4015 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 1820 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   memset((yyval.state), 0, sizeof((yyval.state)));
	   (yyval.state)[0] = state->state_param_enum;
	   (yyval.state)[1] = STATE_LOCAL;
	   (yyval.state)[2] = (yyvsp[-1].integer);
	   (yyval.state)[3] = (yyvsp[-1].integer);
	}
#line 4027 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 1830 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((unsigned) (yyvsp[0].integer) >= state->limits->MaxEnvParams) {
	      yyerror(& (yylsp[0]), state, "invalid environment parameter reference");
	      YYERROR;
	   }
	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 4039 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 1840 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((unsigned) (yyvsp[0].integer) >= state->limits->MaxLocalParams) {
	      yyerror(& (yylsp[0]), state, "invalid local parameter reference");
	      YYERROR;
	   }
	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 4051 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 1855 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.vector).count = 4;
	   (yyval.vector).data[0].f = (yyvsp[0].real);
	   (yyval.vector).data[1].f = (yyvsp[0].real);
	   (yyval.vector).data[2].f = (yyvsp[0].real);
	   (yyval.vector).data[3].f = (yyvsp[0].real);
	}
#line 4063 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 1865 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.vector).count = 1;
	   (yyval.vector).data[0].f = (yyvsp[0].real);
	   (yyval.vector).data[1].f = (yyvsp[0].real);
	   (yyval.vector).data[2].f = (yyvsp[0].real);
	   (yyval.vector).data[3].f = (yyvsp[0].real);
	}
#line 4075 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 1873 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.vector).count = 1;
	   (yyval.vector).data[0].f = (float) (yyvsp[0].integer);
	   (yyval.vector).data[1].f = (float) (yyvsp[0].integer);
	   (yyval.vector).data[2].f = (float) (yyvsp[0].integer);
	   (yyval.vector).data[3].f = (float) (yyvsp[0].integer);
	}
#line 4087 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 1883 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.vector).count = 4;
	   (yyval.vector).data[0].f = (yyvsp[-1].real);
	   (yyval.vector).data[1].f = 0.0f;
	   (yyval.vector).data[2].f = 0.0f;
	   (yyval.vector).data[3].f = 1.0f;
	}
#line 4099 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 1891 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.vector).count = 4;
	   (yyval.vector).data[0].f = (yyvsp[-3].real);
	   (yyval.vector).data[1].f = (yyvsp[-1].real);
	   (yyval.vector).data[2].f = 0.0f;
	   (yyval.vector).data[3].f = 1.0f;
	}
#line 4111 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 1900 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.vector).count = 4;
	   (yyval.vector).data[0].f = (yyvsp[-5].real);
	   (yyval.vector).data[1].f = (yyvsp[-3].real);
	   (yyval.vector).data[2].f = (yyvsp[-1].real);
	   (yyval.vector).data[3].f = 1.0f;
	}
#line 4123 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 1909 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.vector).count = 4;
	   (yyval.vector).data[0].f = (yyvsp[-7].real);
	   (yyval.vector).data[1].f = (yyvsp[-5].real);
	   (yyval.vector).data[2].f = (yyvsp[-3].real);
	   (yyval.vector).data[3].f = (yyvsp[-1].real);
	}
#line 4135 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 1919 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.real) = ((yyvsp[-1].negate)) ? -(yyvsp[0].real) : (yyvsp[0].real);
	}
#line 4143 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 1923 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.real) = (float)(((yyvsp[-1].negate)) ? -(yyvsp[0].integer) : (yyvsp[0].integer));
	}
#line 4151 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 1928 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.negate) = FALSE; }
#line 4157 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 1929 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.negate) = TRUE;  }
#line 4163 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 1930 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.negate) = FALSE; }
#line 4169 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 1933 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = (yyvsp[0].integer); }
#line 4175 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1937 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   /* NV_fragment_program_option defines the size qualifiers in a
	    * fairly broken way.  "SHORT" or "LONG" can optionally be used
	    * before TEMP or OUTPUT.  However, neither is a reserved word!
	    * This means that we have to parse it as an identifier, then check
	    * to make sure it's one of the valid values.  *sigh*
	    *
	    * In addition, the grammar in the extension spec does *not* allow
	    * the size specifier to be optional, but all known implementations
	    * do.
	    */
	   if (!state->option.NV_fragment) {
	      yyerror(& (yylsp[0]), state, "unexpected IDENTIFIER");
	      YYERROR;
	   }

	   if (strcmp("SHORT", (yyvsp[0].string)) == 0) {
	   } else if (strcmp("LONG", (yyvsp[0].string)) == 0) {
	   } else {
	      char *const err_str =
		 make_error_string("invalid storage size specifier \"%s\"",
				   (yyvsp[0].string));

	      yyerror(& (yylsp[0]), state, (err_str != NULL)
		      ? err_str : "invalid storage size specifier");

	      if (err_str != NULL) {
		 free(err_str);
	      }

	      YYERROR;
	   }
	}
#line 4213 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 1971 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	}
#line 4220 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1975 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = (yyvsp[0].integer); }
#line 4226 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 1979 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (!declare_variable(state, (yyvsp[0].string), (yyvsp[-3].integer), & (yylsp[0]))) {
	      free((yyvsp[0].string));
	      YYERROR;
	   }
	}
#line 4237 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 1986 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (!declare_variable(state, (yyvsp[0].string), (yyvsp[-1].integer), & (yylsp[0]))) {
	      free((yyvsp[0].string));
	      YYERROR;
	   }
	}
#line 4248 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 1995 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   struct asm_symbol *const s =
	      declare_variable(state, (yyvsp[-2].string), at_output, & (yylsp[-2]));

	   if (s == NULL) {
	      free((yyvsp[-2].string));
	      YYERROR;
	   } else {
	      s->output_binding = (yyvsp[0].result);
	   }
	}
#line 4264 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 2009 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_vertex) {
	      (yyval.result) = VARYING_SLOT_POS;
	   } else {
	      yyerror(& (yylsp[0]), state, "invalid program result name");
	      YYERROR;
	   }
	}
#line 4277 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 253:
#line 2018 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_vertex) {
	      (yyval.result) = VARYING_SLOT_FOGC;
	   } else {
	      yyerror(& (yylsp[0]), state, "invalid program result name");
	      YYERROR;
	   }
	}
#line 4290 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 2027 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.result) = (yyvsp[0].result);
	}
#line 4298 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 2031 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_vertex) {
	      (yyval.result) = VARYING_SLOT_PSIZ;
	   } else {
	      yyerror(& (yylsp[0]), state, "invalid program result name");
	      YYERROR;
	   }
	}
#line 4311 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 2040 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_vertex) {
	      (yyval.result) = VARYING_SLOT_TEX0 + (yyvsp[0].integer);
	   } else {
	      yyerror(& (yylsp[-1]), state, "invalid program result name");
	      YYERROR;
	   }
	}
#line 4324 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 2049 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_fragment) {
	      (yyval.result) = FRAG_RESULT_DEPTH;
	   } else {
	      yyerror(& (yylsp[0]), state, "invalid program result name");
	      YYERROR;
	   }
	}
#line 4337 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 2060 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.result) = (yyvsp[-1].integer) + (yyvsp[0].integer);
	}
#line 4345 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 2066 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_vertex) {
	      (yyval.integer) = VARYING_SLOT_COL0;
	   } else {
	      if (state->option.DrawBuffers)
		 (yyval.integer) = FRAG_RESULT_DATA0;
	      else
		 (yyval.integer) = FRAG_RESULT_COLOR;
	   }
	}
#line 4360 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 2077 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_vertex) {
	      yyerror(& (yylsp[-2]), state, "invalid program result name");
	      YYERROR;
	   } else {
	      if (!state->option.DrawBuffers) {
		 /* From the ARB_draw_buffers spec (same text exists
		  * for ATI_draw_buffers):
		  *
		  *     If this option is not specified, a fragment
		  *     program that attempts to bind
		  *     "result.color[n]" will fail to load, and only
		  *     "result.color" will be allowed.
		  */
		 yyerror(& (yylsp[-2]), state,
			 "result.color[] used without "
			 "`OPTION ARB_draw_buffers' or "
			 "`OPTION ATI_draw_buffers'");
		 YYERROR;
	      } else if ((yyvsp[-1].integer) >= state->MaxDrawBuffers) {
		 yyerror(& (yylsp[-2]), state,
			 "result.color[] exceeds MAX_DRAW_BUFFERS_ARB");
		 YYERROR;
	      }
	      (yyval.integer) = FRAG_RESULT_DATA0 + (yyvsp[-1].integer);
	   }
	}
#line 4392 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 2105 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_vertex) {
	      (yyval.integer) = VARYING_SLOT_COL0;
	   } else {
	      yyerror(& (yylsp[0]), state, "invalid program result name");
	      YYERROR;
	   }
	}
#line 4405 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 2114 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_vertex) {
	      (yyval.integer) = VARYING_SLOT_BFC0;
	   } else {
	      yyerror(& (yylsp[0]), state, "invalid program result name");
	      YYERROR;
	   }
	}
#line 4418 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 2125 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   (yyval.integer) = 0; 
	}
#line 4426 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 2129 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_vertex) {
	      (yyval.integer) = 0;
	   } else {
	      yyerror(& (yylsp[0]), state, "invalid program result name");
	      YYERROR;
	   }
	}
#line 4439 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 2138 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if (state->mode == ARB_vertex) {
	      (yyval.integer) = 1;
	   } else {
	      yyerror(& (yylsp[0]), state, "invalid program result name");
	      YYERROR;
	   }
	}
#line 4452 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 2148 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = 0; }
#line 4458 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 2149 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = 0; }
#line 4464 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 2150 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = 1; }
#line 4470 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 2153 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = 0; }
#line 4476 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 2154 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = 0; }
#line 4482 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 2155 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = 1; }
#line 4488 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 2158 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = 0; }
#line 4494 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 2159 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = (yyvsp[-1].integer); }
#line 4500 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 2162 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = 0; }
#line 4506 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 275:
#line 2163 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = (yyvsp[-1].integer); }
#line 4512 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 276:
#line 2166 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = 0; }
#line 4518 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 2167 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    { (yyval.integer) = (yyvsp[-1].integer); }
#line 4524 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 2171 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((unsigned) (yyvsp[0].integer) >= state->MaxTextureCoordUnits) {
	      yyerror(& (yylsp[0]), state, "invalid texture coordinate unit selector");
	      YYERROR;
	   }

	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 4537 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 2182 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((unsigned) (yyvsp[0].integer) >= state->MaxTextureImageUnits) {
	      yyerror(& (yylsp[0]), state, "invalid texture image unit selector");
	      YYERROR;
	   }

	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 4550 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 2193 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   if ((unsigned) (yyvsp[0].integer) >= state->MaxTextureUnits) {
	      yyerror(& (yylsp[0]), state, "invalid texture unit selector");
	      YYERROR;
	   }

	   (yyval.integer) = (yyvsp[0].integer);
	}
#line 4563 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 2204 "../../../src/mesa/program/program_parse.y" /* yacc.c:1646  */
    {
	   struct asm_symbol *exist = (struct asm_symbol *)
	      _mesa_symbol_table_find_symbol(state->st, 0, (yyvsp[-2].string));
	   struct asm_symbol *target = (struct asm_symbol *)
	      _mesa_symbol_table_find_symbol(state->st, 0, (yyvsp[0].string));

	   free((yyvsp[0].string));

	   if (exist != NULL) {
	      char m[1000];
	      _mesa_snprintf(m, sizeof(m), "redeclared identifier: %s", (yyvsp[-2].string));
	      free((yyvsp[-2].string));
	      yyerror(& (yylsp[-2]), state, m);
	      YYERROR;
	   } else if (target == NULL) {
	      free((yyvsp[-2].string));
	      yyerror(& (yylsp[0]), state,
		      "undefined variable binding in ALIAS statement");
	      YYERROR;
	   } else {
	      _mesa_symbol_table_add_symbol(state->st, 0, (yyvsp[-2].string), target);
	   }
	}
#line 4591 "program/program_parse.tab.c" /* yacc.c:1646  */
    break;


#line 4595 "program/program_parse.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, state, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, state, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, state);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, state);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, state, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, state);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, state);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 2233 "../../../src/mesa/program/program_parse.y" /* yacc.c:1906  */


void
asm_instruction_set_operands(struct asm_instruction *inst,
			     const struct prog_dst_register *dst,
			     const struct asm_src_register *src0,
			     const struct asm_src_register *src1,
			     const struct asm_src_register *src2)
{
   /* In the core ARB extensions only the KIL instruction doesn't have a
    * destination register.
    */
   if (dst == NULL) {
      init_dst_reg(& inst->Base.DstReg);
   } else {
      inst->Base.DstReg = *dst;
   }

   /* The only instruction that doesn't have any source registers is the
    * condition-code based KIL instruction added by NV_fragment_program_option.
    */
   if (src0 != NULL) {
      inst->Base.SrcReg[0] = src0->Base;
      inst->SrcReg[0] = *src0;
   } else {
      init_src_reg(& inst->SrcReg[0]);
   }

   if (src1 != NULL) {
      inst->Base.SrcReg[1] = src1->Base;
      inst->SrcReg[1] = *src1;
   } else {
      init_src_reg(& inst->SrcReg[1]);
   }

   if (src2 != NULL) {
      inst->Base.SrcReg[2] = src2->Base;
      inst->SrcReg[2] = *src2;
   } else {
      init_src_reg(& inst->SrcReg[2]);
   }
}


struct asm_instruction *
asm_instruction_ctor(gl_inst_opcode op,
		     const struct prog_dst_register *dst,
		     const struct asm_src_register *src0,
		     const struct asm_src_register *src1,
		     const struct asm_src_register *src2)
{
   struct asm_instruction *inst = CALLOC_STRUCT(asm_instruction);

   if (inst) {
      _mesa_init_instructions(& inst->Base, 1);
      inst->Base.Opcode = op;

      asm_instruction_set_operands(inst, dst, src0, src1, src2);
   }

   return inst;
}


struct asm_instruction *
asm_instruction_copy_ctor(const struct prog_instruction *base,
			  const struct prog_dst_register *dst,
			  const struct asm_src_register *src0,
			  const struct asm_src_register *src1,
			  const struct asm_src_register *src2)
{
   struct asm_instruction *inst = CALLOC_STRUCT(asm_instruction);

   if (inst) {
      _mesa_init_instructions(& inst->Base, 1);
      inst->Base.Opcode = base->Opcode;
      inst->Base.CondUpdate = base->CondUpdate;
      inst->Base.CondDst = base->CondDst;
      inst->Base.SaturateMode = base->SaturateMode;
      inst->Base.Precision = base->Precision;

      asm_instruction_set_operands(inst, dst, src0, src1, src2);
   }

   return inst;
}


void
init_dst_reg(struct prog_dst_register *r)
{
   memset(r, 0, sizeof(*r));
   r->File = PROGRAM_UNDEFINED;
   r->WriteMask = WRITEMASK_XYZW;
   r->CondMask = COND_TR;
   r->CondSwizzle = SWIZZLE_NOOP;
}


/** Like init_dst_reg() but set the File and Index fields. */
void
set_dst_reg(struct prog_dst_register *r, gl_register_file file, GLint index)
{
   const GLint maxIndex = 1 << INST_INDEX_BITS;
   const GLint minIndex = 0;
   assert(index >= minIndex);
   (void) minIndex;
   assert(index <= maxIndex);
   (void) maxIndex;
   assert(file == PROGRAM_TEMPORARY ||
	  file == PROGRAM_ADDRESS ||
	  file == PROGRAM_OUTPUT);
   memset(r, 0, sizeof(*r));
   r->File = file;
   r->Index = index;
   r->WriteMask = WRITEMASK_XYZW;
   r->CondMask = COND_TR;
   r->CondSwizzle = SWIZZLE_NOOP;
}


void
init_src_reg(struct asm_src_register *r)
{
   memset(r, 0, sizeof(*r));
   r->Base.File = PROGRAM_UNDEFINED;
   r->Base.Swizzle = SWIZZLE_NOOP;
   r->Symbol = NULL;
}


/** Like init_src_reg() but set the File and Index fields.
 * \return GL_TRUE if a valid src register, GL_FALSE otherwise
 */
void
set_src_reg(struct asm_src_register *r, gl_register_file file, GLint index)
{
   set_src_reg_swz(r, file, index, SWIZZLE_XYZW);
}


void
set_src_reg_swz(struct asm_src_register *r, gl_register_file file, GLint index,
                GLuint swizzle)
{
   const GLint maxIndex = (1 << INST_INDEX_BITS) - 1;
   const GLint minIndex = -(1 << INST_INDEX_BITS);
   assert(file < PROGRAM_FILE_MAX);
   assert(index >= minIndex);
   (void) minIndex;
   assert(index <= maxIndex);
   (void) maxIndex;
   memset(r, 0, sizeof(*r));
   r->Base.File = file;
   r->Base.Index = index;
   r->Base.Swizzle = swizzle;
   r->Symbol = NULL;
}


/**
 * Validate the set of inputs used by a program
 *
 * Validates that legal sets of inputs are used by the program.  In this case
 * "used" included both reading the input or binding the input to a name using
 * the \c ATTRIB command.
 *
 * \return
 * \c TRUE if the combination of inputs used is valid, \c FALSE otherwise.
 */
int
validate_inputs(struct YYLTYPE *locp, struct asm_parser_state *state)
{
   const GLbitfield64 inputs = state->prog->InputsRead | state->InputsBound;

   if (((inputs & VERT_BIT_FF_ALL) & (inputs >> VERT_ATTRIB_GENERIC0)) != 0) {
      yyerror(locp, state, "illegal use of generic attribute and name attribute");
      return 0;
   }

   return 1;
}


struct asm_symbol *
declare_variable(struct asm_parser_state *state, char *name, enum asm_type t,
		 struct YYLTYPE *locp)
{
   struct asm_symbol *s = NULL;
   struct asm_symbol *exist = (struct asm_symbol *)
      _mesa_symbol_table_find_symbol(state->st, 0, name);


   if (exist != NULL) {
      yyerror(locp, state, "redeclared identifier");
   } else {
      s = calloc(1, sizeof(struct asm_symbol));
      s->name = name;
      s->type = t;

      switch (t) {
      case at_temp:
	 if (state->prog->NumTemporaries >= state->limits->MaxTemps) {
	    yyerror(locp, state, "too many temporaries declared");
	    free(s);
	    return NULL;
	 }

	 s->temp_binding = state->prog->NumTemporaries;
	 state->prog->NumTemporaries++;
	 break;

      case at_address:
	 if (state->prog->NumAddressRegs >= state->limits->MaxAddressRegs) {
	    yyerror(locp, state, "too many address registers declared");
	    free(s);
	    return NULL;
	 }

	 /* FINISHME: Add support for multiple address registers.
	  */
	 state->prog->NumAddressRegs++;
	 break;

      default:
	 break;
      }

      _mesa_symbol_table_add_symbol(state->st, 0, s->name, s);
      s->next = state->sym;
      state->sym = s;
   }

   return s;
}


int add_state_reference(struct gl_program_parameter_list *param_list,
			const gl_state_index tokens[STATE_LENGTH])
{
   const GLuint size = 4; /* XXX fix */
   char *name;
   GLint index;

   name = _mesa_program_state_string(tokens);
   index = _mesa_add_parameter(param_list, PROGRAM_STATE_VAR, name,
                               size, GL_NONE, NULL, tokens);
   param_list->StateFlags |= _mesa_program_state_flags(tokens);

   /* free name string here since we duplicated it in add_parameter() */
   free(name);

   return index;
}


int
initialize_symbol_from_state(struct gl_program *prog,
			     struct asm_symbol *param_var, 
			     const gl_state_index tokens[STATE_LENGTH])
{
   int idx = -1;
   gl_state_index state_tokens[STATE_LENGTH];


   memcpy(state_tokens, tokens, sizeof(state_tokens));

   param_var->type = at_param;
   param_var->param_binding_type = PROGRAM_STATE_VAR;

   /* If we are adding a STATE_MATRIX that has multiple rows, we need to
    * unroll it and call add_state_reference() for each row
    */
   if ((state_tokens[0] == STATE_MODELVIEW_MATRIX ||
	state_tokens[0] == STATE_PROJECTION_MATRIX ||
	state_tokens[0] == STATE_MVP_MATRIX ||
	state_tokens[0] == STATE_TEXTURE_MATRIX ||
	state_tokens[0] == STATE_PROGRAM_MATRIX)
       && (state_tokens[2] != state_tokens[3])) {
      int row;
      const int first_row = state_tokens[2];
      const int last_row = state_tokens[3];

      for (row = first_row; row <= last_row; row++) {
	 state_tokens[2] = state_tokens[3] = row;

	 idx = add_state_reference(prog->Parameters, state_tokens);
	 if (param_var->param_binding_begin == ~0U) {
	    param_var->param_binding_begin = idx;
            param_var->param_binding_swizzle = SWIZZLE_XYZW;
         }

	 param_var->param_binding_length++;
      }
   }
   else {
      idx = add_state_reference(prog->Parameters, state_tokens);
      if (param_var->param_binding_begin == ~0U) {
	 param_var->param_binding_begin = idx;
         param_var->param_binding_swizzle = SWIZZLE_XYZW;
      }
      param_var->param_binding_length++;
   }

   return idx;
}


int
initialize_symbol_from_param(struct gl_program *prog,
			     struct asm_symbol *param_var, 
			     const gl_state_index tokens[STATE_LENGTH])
{
   int idx = -1;
   gl_state_index state_tokens[STATE_LENGTH];


   memcpy(state_tokens, tokens, sizeof(state_tokens));

   assert((state_tokens[0] == STATE_VERTEX_PROGRAM)
	  || (state_tokens[0] == STATE_FRAGMENT_PROGRAM));
   assert((state_tokens[1] == STATE_ENV)
	  || (state_tokens[1] == STATE_LOCAL));

   /*
    * The param type is STATE_VAR.  The program parameter entry will
    * effectively be a pointer into the LOCAL or ENV parameter array.
    */
   param_var->type = at_param;
   param_var->param_binding_type = PROGRAM_STATE_VAR;

   /* If we are adding a STATE_ENV or STATE_LOCAL that has multiple elements,
    * we need to unroll it and call add_state_reference() for each row
    */
   if (state_tokens[2] != state_tokens[3]) {
      int row;
      const int first_row = state_tokens[2];
      const int last_row = state_tokens[3];

      for (row = first_row; row <= last_row; row++) {
	 state_tokens[2] = state_tokens[3] = row;

	 idx = add_state_reference(prog->Parameters, state_tokens);
	 if (param_var->param_binding_begin == ~0U) {
	    param_var->param_binding_begin = idx;
            param_var->param_binding_swizzle = SWIZZLE_XYZW;
         }
	 param_var->param_binding_length++;
      }
   }
   else {
      idx = add_state_reference(prog->Parameters, state_tokens);
      if (param_var->param_binding_begin == ~0U) {
	 param_var->param_binding_begin = idx;
         param_var->param_binding_swizzle = SWIZZLE_XYZW;
      }
      param_var->param_binding_length++;
   }

   return idx;
}


/**
 * Put a float/vector constant/literal into the parameter list.
 * \param param_var  returns info about the parameter/constant's location,
 *                   binding, type, etc.
 * \param vec  the vector/constant to add
 * \param allowSwizzle  if true, try to consolidate constants which only differ
 *                      by a swizzle.  We don't want to do this when building
 *                      arrays of constants that may be indexed indirectly.
 * \return index of the constant in the parameter list.
 */
int
initialize_symbol_from_const(struct gl_program *prog,
			     struct asm_symbol *param_var, 
			     const struct asm_vector *vec,
                             GLboolean allowSwizzle)
{
   unsigned swizzle;
   const int idx = _mesa_add_unnamed_constant(prog->Parameters,
                                              vec->data, vec->count,
                                              allowSwizzle ? &swizzle : NULL);

   param_var->type = at_param;
   param_var->param_binding_type = PROGRAM_CONSTANT;

   if (param_var->param_binding_begin == ~0U) {
      param_var->param_binding_begin = idx;
      param_var->param_binding_swizzle = allowSwizzle ? swizzle : SWIZZLE_XYZW;
   }
   param_var->param_binding_length++;

   return idx;
}


char *
make_error_string(const char *fmt, ...)
{
   int length;
   char *str;
   va_list args;


   /* Call vsnprintf once to determine how large the final string is.  Call it
    * again to do the actual formatting.  from the vsnprintf manual page:
    *
    *    Upon successful return, these functions return the number of
    *    characters printed  (not including the trailing '\0' used to end
    *    output to strings).
    */
   va_start(args, fmt);
   length = 1 + vsnprintf(NULL, 0, fmt, args);
   va_end(args);

   str = malloc(length);
   if (str) {
      va_start(args, fmt);
      vsnprintf(str, length, fmt, args);
      va_end(args);
   }

   return str;
}


void
yyerror(YYLTYPE *locp, struct asm_parser_state *state, const char *s)
{
   char *err_str;


   err_str = make_error_string("glProgramStringARB(%s)\n", s);
   if (err_str) {
      _mesa_error(state->ctx, GL_INVALID_OPERATION, "%s", err_str);
      free(err_str);
   }

   err_str = make_error_string("line %u, char %u: error: %s\n",
			       locp->first_line, locp->first_column, s);
   _mesa_set_program_error(state->ctx, locp->position, err_str);

   if (err_str) {
      free(err_str);
   }
}


GLboolean
_mesa_parse_arb_program(struct gl_context *ctx, GLenum target, const GLubyte *str,
			GLsizei len, struct asm_parser_state *state)
{
   struct asm_instruction *inst;
   unsigned i;
   GLubyte *strz;
   GLboolean result = GL_FALSE;
   void *temp;
   struct asm_symbol *sym;

   state->ctx = ctx;
   state->prog->Target = target;
   state->prog->Parameters = _mesa_new_parameter_list();

   /* Make a copy of the program string and force it to be NUL-terminated.
    */
   strz = (GLubyte *) malloc(len + 1);
   if (strz == NULL) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glProgramStringARB");
      return GL_FALSE;
   }
   memcpy (strz, str, len);
   strz[len] = '\0';

   state->prog->String = strz;

   state->st = _mesa_symbol_table_ctor();

   state->limits = (target == GL_VERTEX_PROGRAM_ARB)
      ? & ctx->Const.Program[MESA_SHADER_VERTEX]
      : & ctx->Const.Program[MESA_SHADER_FRAGMENT];

   state->MaxTextureImageUnits = ctx->Const.Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits;
   state->MaxTextureCoordUnits = ctx->Const.MaxTextureCoordUnits;
   state->MaxTextureUnits = ctx->Const.MaxTextureUnits;
   state->MaxClipPlanes = ctx->Const.MaxClipPlanes;
   state->MaxLights = ctx->Const.MaxLights;
   state->MaxProgramMatrices = ctx->Const.MaxProgramMatrices;
   state->MaxDrawBuffers = ctx->Const.MaxDrawBuffers;

   state->state_param_enum = (target == GL_VERTEX_PROGRAM_ARB)
      ? STATE_VERTEX_PROGRAM : STATE_FRAGMENT_PROGRAM;

   _mesa_set_program_error(ctx, -1, NULL);

   _mesa_program_lexer_ctor(& state->scanner, state, (const char *) str, len);
   yyparse(state);
   _mesa_program_lexer_dtor(state->scanner);


   if (ctx->Program.ErrorPos != -1) {
      goto error;
   }

   if (! _mesa_layout_parameters(state)) {
      struct YYLTYPE loc;

      loc.first_line = 0;
      loc.first_column = 0;
      loc.position = len;

      yyerror(& loc, state, "invalid PARAM usage");
      goto error;
   }


   
   /* Add one instruction to store the "END" instruction.
    */
   state->prog->Instructions =
      _mesa_alloc_instructions(state->prog->NumInstructions + 1);

   if (state->prog->Instructions == NULL) {
      goto error;
   }

   inst = state->inst_head;
   for (i = 0; i < state->prog->NumInstructions; i++) {
      struct asm_instruction *const temp = inst->next;

      state->prog->Instructions[i] = inst->Base;
      inst = temp;
   }

   /* Finally, tag on an OPCODE_END instruction */
   {
      const GLuint numInst = state->prog->NumInstructions;
      _mesa_init_instructions(state->prog->Instructions + numInst, 1);
      state->prog->Instructions[numInst].Opcode = OPCODE_END;
   }
   state->prog->NumInstructions++;

   state->prog->NumParameters = state->prog->Parameters->NumParameters;
   state->prog->NumAttributes = _mesa_bitcount_64(state->prog->InputsRead);

   /*
    * Initialize native counts to logical counts.  The device driver may
    * change them if program is translated into a hardware program.
    */
   state->prog->NumNativeInstructions = state->prog->NumInstructions;
   state->prog->NumNativeTemporaries = state->prog->NumTemporaries;
   state->prog->NumNativeParameters = state->prog->NumParameters;
   state->prog->NumNativeAttributes = state->prog->NumAttributes;
   state->prog->NumNativeAddressRegs = state->prog->NumAddressRegs;

   result = GL_TRUE;

error:
   for (inst = state->inst_head; inst != NULL; inst = temp) {
      temp = inst->next;
      free(inst);
   }

   state->inst_head = NULL;
   state->inst_tail = NULL;

   for (sym = state->sym; sym != NULL; sym = temp) {
      temp = sym->next;

      free((void *) sym->name);
      free(sym);
   }
   state->sym = NULL;

   _mesa_symbol_table_dtor(state->st);
   state->st = NULL;

   return result;
}
