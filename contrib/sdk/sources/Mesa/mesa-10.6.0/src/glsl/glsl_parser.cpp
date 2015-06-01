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
#define yyparse         _mesa_glsl_parse
#define yylex           _mesa_glsl_lex
#define yyerror         _mesa_glsl_error
#define yydebug         _mesa_glsl_debug
#define yynerrs         _mesa_glsl_nerrs


/* Copy the first part of user declarations.  */
#line 1 "../../../src/glsl/glsl_parser.yy" /* yacc.c:339  */

/*
 * Copyright © 2008, 2009 Intel Corporation
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <strings.h>
#endif
#include <assert.h>

#include "ast.h"
#include "glsl_parser_extras.h"
#include "glsl_types.h"
#include "main/context.h"

#ifdef _MSC_VER
#pragma warning( disable : 4065 ) // switch statement contains 'default' but no 'case' labels
#endif

#undef yyerror

static void yyerror(YYLTYPE *loc, _mesa_glsl_parse_state *st, const char *msg)
{
   _mesa_glsl_error(loc, st, "%s", msg);
}

static int
_mesa_glsl_lex(YYSTYPE *val, YYLTYPE *loc, _mesa_glsl_parse_state *state)
{
   return _mesa_glsl_lexer_lex(val, loc, state->scanner);
}

static bool match_layout_qualifier(const char *s1, const char *s2,
                                   _mesa_glsl_parse_state *state)
{
   /* From the GLSL 1.50 spec, section 4.3.8 (Layout Qualifiers):
    *
    *     "The tokens in any layout-qualifier-id-list ... are not case
    *     sensitive, unless explicitly noted otherwise."
    *
    * The text "unless explicitly noted otherwise" appears to be
    * vacuous--no desktop GLSL spec (up through GLSL 4.40) notes
    * otherwise.
    *
    * However, the GLSL ES 3.00 spec says, in section 4.3.8 (Layout
    * Qualifiers):
    *
    *     "As for other identifiers, they are case sensitive."
    *
    * So we need to do a case-sensitive or a case-insensitive match,
    * depending on whether we are compiling for GLSL ES.
    */
   if (state->es_shader)
      return strcmp(s1, s2);
   else
      return strcasecmp(s1, s2);
}

#line 152 "glsl_parser.cpp" /* yacc.c:339  */

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
   by #include "glsl_parser.h".  */
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
#line 98 "../../../src/glsl/glsl_parser.yy" /* yacc.c:355  */

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

#line 473 "glsl_parser.cpp" /* yacc.c:355  */
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

/* Copy the second part of user declarations.  */

#line 503 "glsl_parser.cpp" /* yacc.c:358  */

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
#define YYLAST   5744

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  267
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  109
/* YYNRULES -- Number of rules.  */
#define YYNRULES  402
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  557

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   497

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   251,     2,     2,     2,   255,   258,     2,
     243,   244,   253,   249,   248,   250,   247,   254,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   262,   264,
     256,   263,   257,   261,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   245,     2,   246,   259,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   265,   260,   266,   252,     2,     2,     2,
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
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   308,   308,   307,   319,   321,   328,   338,   339,   340,
     341,   342,   366,   368,   372,   373,   374,   378,   387,   395,
     403,   414,   415,   419,   426,   433,   440,   447,   454,   461,
     468,   469,   475,   479,   486,   492,   501,   505,   509,   510,
     519,   520,   524,   525,   529,   535,   547,   551,   557,   565,
     576,   577,   581,   582,   586,   592,   604,   616,   617,   623,
     629,   639,   640,   641,   642,   646,   647,   653,   659,   668,
     669,   675,   684,   685,   691,   700,   701,   707,   713,   719,
     728,   729,   735,   744,   745,   754,   755,   764,   765,   774,
     775,   784,   785,   794,   795,   804,   805,   814,   815,   824,
     825,   826,   827,   828,   829,   830,   831,   832,   833,   834,
     838,   842,   858,   862,   867,   871,   876,   883,   887,   888,
     892,   897,   905,   919,   929,   943,   948,   962,   965,   973,
     981,   993,  1004,  1011,  1016,  1021,  1030,  1034,  1035,  1045,
    1055,  1065,  1079,  1086,  1096,  1106,  1116,  1126,  1138,  1153,
    1160,  1171,  1178,  1179,  1189,  1190,  1194,  1394,  1533,  1559,
    1564,  1572,  1577,  1582,  1591,  1596,  1601,  1602,  1603,  1604,
    1605,  1606,  1624,  1632,  1655,  1679,  1696,  1712,  1732,  1743,
    1751,  1756,  1764,  1769,  1774,  1779,  1784,  1803,  1811,  1816,
    1821,  1827,  1832,  1840,  1846,  1852,  1866,  1881,  1882,  1890,
    1896,  1902,  1911,  1912,  1913,  1914,  1915,  1916,  1917,  1918,
    1919,  1920,  1921,  1922,  1923,  1924,  1925,  1926,  1927,  1928,
    1929,  1930,  1931,  1932,  1933,  1934,  1935,  1936,  1937,  1938,
    1939,  1940,  1941,  1942,  1943,  1944,  1945,  1946,  1947,  1948,
    1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,  1957,  1958,
    1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,  1967,  1968,
    1969,  1970,  1971,  1972,  1973,  1974,  1975,  1976,  1977,  1978,
    1979,  1980,  1981,  1982,  1983,  1984,  1985,  1986,  1987,  1988,
    1989,  1990,  1991,  1992,  1993,  1994,  1995,  1996,  1997,  1998,
    1999,  2000,  2001,  2002,  2003,  2004,  2005,  2006,  2007,  2008,
    2009,  2010,  2011,  2012,  2013,  2014,  2015,  2016,  2017,  2018,
    2019,  2020,  2021,  2022,  2023,  2024,  2028,  2033,  2038,  2046,
    2053,  2062,  2067,  2075,  2094,  2099,  2107,  2113,  2122,  2123,
    2127,  2134,  2141,  2148,  2154,  2155,  2159,  2160,  2161,  2162,
    2163,  2164,  2168,  2175,  2174,  2188,  2189,  2193,  2199,  2208,
    2218,  2230,  2236,  2245,  2254,  2259,  2267,  2271,  2289,  2297,
    2302,  2310,  2315,  2323,  2331,  2339,  2347,  2355,  2363,  2371,
    2378,  2385,  2395,  2396,  2400,  2402,  2408,  2413,  2422,  2428,
    2434,  2440,  2446,  2455,  2456,  2457,  2458,  2462,  2476,  2480,
    2503,  2630,  2635,  2640,  2649,  2653,  2659,  2668,  2673,  2681,
    2705,  2713,  2721
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ATTRIBUTE", "CONST_TOK", "BOOL_TOK",
  "FLOAT_TOK", "INT_TOK", "UINT_TOK", "DOUBLE_TOK", "BREAK", "CONTINUE",
  "DO", "ELSE", "FOR", "IF", "DISCARD", "RETURN", "SWITCH", "CASE",
  "DEFAULT", "BVEC2", "BVEC3", "BVEC4", "IVEC2", "IVEC3", "IVEC4", "UVEC2",
  "UVEC3", "UVEC4", "VEC2", "VEC3", "VEC4", "DVEC2", "DVEC3", "DVEC4",
  "CENTROID", "IN_TOK", "OUT_TOK", "INOUT_TOK", "UNIFORM", "VARYING",
  "SAMPLE", "NOPERSPECTIVE", "FLAT", "SMOOTH", "MAT2X2", "MAT2X3",
  "MAT2X4", "MAT3X2", "MAT3X3", "MAT3X4", "MAT4X2", "MAT4X3", "MAT4X4",
  "DMAT2X2", "DMAT2X3", "DMAT2X4", "DMAT3X2", "DMAT3X3", "DMAT3X4",
  "DMAT4X2", "DMAT4X3", "DMAT4X4", "SAMPLER1D", "SAMPLER2D", "SAMPLER3D",
  "SAMPLERCUBE", "SAMPLER1DSHADOW", "SAMPLER2DSHADOW", "SAMPLERCUBESHADOW",
  "SAMPLER1DARRAY", "SAMPLER2DARRAY", "SAMPLER1DARRAYSHADOW",
  "SAMPLER2DARRAYSHADOW", "SAMPLERCUBEARRAY", "SAMPLERCUBEARRAYSHADOW",
  "ISAMPLER1D", "ISAMPLER2D", "ISAMPLER3D", "ISAMPLERCUBE",
  "ISAMPLER1DARRAY", "ISAMPLER2DARRAY", "ISAMPLERCUBEARRAY", "USAMPLER1D",
  "USAMPLER2D", "USAMPLER3D", "USAMPLERCUBE", "USAMPLER1DARRAY",
  "USAMPLER2DARRAY", "USAMPLERCUBEARRAY", "SAMPLER2DRECT",
  "ISAMPLER2DRECT", "USAMPLER2DRECT", "SAMPLER2DRECTSHADOW",
  "SAMPLERBUFFER", "ISAMPLERBUFFER", "USAMPLERBUFFER", "SAMPLER2DMS",
  "ISAMPLER2DMS", "USAMPLER2DMS", "SAMPLER2DMSARRAY", "ISAMPLER2DMSARRAY",
  "USAMPLER2DMSARRAY", "SAMPLEREXTERNALOES", "IMAGE1D", "IMAGE2D",
  "IMAGE3D", "IMAGE2DRECT", "IMAGECUBE", "IMAGEBUFFER", "IMAGE1DARRAY",
  "IMAGE2DARRAY", "IMAGECUBEARRAY", "IMAGE2DMS", "IMAGE2DMSARRAY",
  "IIMAGE1D", "IIMAGE2D", "IIMAGE3D", "IIMAGE2DRECT", "IIMAGECUBE",
  "IIMAGEBUFFER", "IIMAGE1DARRAY", "IIMAGE2DARRAY", "IIMAGECUBEARRAY",
  "IIMAGE2DMS", "IIMAGE2DMSARRAY", "UIMAGE1D", "UIMAGE2D", "UIMAGE3D",
  "UIMAGE2DRECT", "UIMAGECUBE", "UIMAGEBUFFER", "UIMAGE1DARRAY",
  "UIMAGE2DARRAY", "UIMAGECUBEARRAY", "UIMAGE2DMS", "UIMAGE2DMSARRAY",
  "IMAGE1DSHADOW", "IMAGE2DSHADOW", "IMAGE1DARRAYSHADOW",
  "IMAGE2DARRAYSHADOW", "COHERENT", "VOLATILE", "RESTRICT", "READONLY",
  "WRITEONLY", "ATOMIC_UINT", "STRUCT", "VOID_TOK", "WHILE", "IDENTIFIER",
  "TYPE_IDENTIFIER", "NEW_IDENTIFIER", "FLOATCONSTANT", "DOUBLECONSTANT",
  "INTCONSTANT", "UINTCONSTANT", "BOOLCONSTANT", "FIELD_SELECTION",
  "LEFT_OP", "RIGHT_OP", "INC_OP", "DEC_OP", "LE_OP", "GE_OP", "EQ_OP",
  "NE_OP", "AND_OP", "OR_OP", "XOR_OP", "MUL_ASSIGN", "DIV_ASSIGN",
  "ADD_ASSIGN", "MOD_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN",
  "XOR_ASSIGN", "OR_ASSIGN", "SUB_ASSIGN", "INVARIANT", "PRECISE", "LOWP",
  "MEDIUMP", "HIGHP", "SUPERP", "PRECISION", "VERSION_TOK", "EXTENSION",
  "LINE", "COLON", "EOL", "INTERFACE", "OUTPUT", "PRAGMA_DEBUG_ON",
  "PRAGMA_DEBUG_OFF", "PRAGMA_OPTIMIZE_ON", "PRAGMA_OPTIMIZE_OFF",
  "PRAGMA_INVARIANT_ALL", "LAYOUT_TOK", "ASM", "CLASS", "UNION", "ENUM",
  "TYPEDEF", "TEMPLATE", "THIS", "PACKED_TOK", "GOTO", "INLINE_TOK",
  "NOINLINE", "PUBLIC_TOK", "STATIC", "EXTERN", "EXTERNAL", "LONG_TOK",
  "SHORT_TOK", "HALF", "FIXED_TOK", "UNSIGNED", "INPUT_TOK", "HVEC2",
  "HVEC3", "HVEC4", "FVEC2", "FVEC3", "FVEC4", "SAMPLER3DRECT", "SIZEOF",
  "CAST", "NAMESPACE", "USING", "RESOURCE", "PATCH", "SUBROUTINE",
  "ERROR_TOK", "COMMON", "PARTITION", "ACTIVE", "FILTER", "ROW_MAJOR",
  "THEN", "'('", "')'", "'['", "']'", "'.'", "','", "'+'", "'-'", "'!'",
  "'~'", "'*'", "'/'", "'%'", "'<'", "'>'", "'&'", "'^'", "'|'", "'?'",
  "':'", "'='", "';'", "'{'", "'}'", "$accept", "translation_unit", "$@1",
  "version_statement", "pragma_statement", "extension_statement_list",
  "any_identifier", "extension_statement", "external_declaration_list",
  "variable_identifier", "primary_expression", "postfix_expression",
  "integer_expression", "function_call", "function_call_or_method",
  "function_call_generic", "function_call_header_no_parameters",
  "function_call_header_with_parameters", "function_call_header",
  "function_identifier", "method_call_generic",
  "method_call_header_no_parameters", "method_call_header_with_parameters",
  "method_call_header", "unary_expression", "unary_operator",
  "multiplicative_expression", "additive_expression", "shift_expression",
  "relational_expression", "equality_expression", "and_expression",
  "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_xor_expression",
  "logical_or_expression", "conditional_expression",
  "assignment_expression", "assignment_operator", "expression",
  "constant_expression", "declaration", "function_prototype",
  "function_declarator", "function_header_with_parameters",
  "function_header", "parameter_declarator", "parameter_declaration",
  "parameter_qualifier", "parameter_direction_qualifier",
  "parameter_type_specifier", "init_declarator_list", "single_declaration",
  "fully_specified_type", "layout_qualifier", "layout_qualifier_id_list",
  "integer_constant", "layout_qualifier_id",
  "interface_block_layout_qualifier", "interpolation_qualifier",
  "type_qualifier", "auxiliary_storage_qualifier", "storage_qualifier",
  "memory_qualifier", "array_specifier", "type_specifier",
  "type_specifier_nonarray", "basic_type_specifier_nonarray",
  "precision_qualifier", "struct_specifier", "struct_declaration_list",
  "struct_declaration", "struct_declarator_list", "struct_declarator",
  "initializer", "initializer_list", "declaration_statement", "statement",
  "simple_statement", "compound_statement", "$@2",
  "statement_no_new_scope", "compound_statement_no_new_scope",
  "statement_list", "expression_statement", "selection_statement",
  "selection_rest_statement", "condition", "switch_statement",
  "switch_body", "case_label", "case_label_list", "case_statement",
  "case_statement_list", "iteration_statement", "for_init_statement",
  "conditionopt", "for_rest_statement", "jump_statement",
  "external_declaration", "function_definition", "interface_block",
  "basic_interface_block", "interface_qualifier", "instance_name_opt",
  "member_list", "member_declaration", "layout_defaults", YY_NULLPTR
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
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,    40,    41,    91,    93,    46,    44,    43,
      45,    33,   126,    42,    47,    37,    60,    62,    38,    94,
     124,    63,    58,    61,    59,   123,   125
};
# endif

#define YYPACT_NINF -408

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-408)))

#define YYTABLE_NINF -394

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -164,  -123,    88,  -408,   -98,  -408,  -153,  -408,  -408,  -408,
    -408,   -91,    71,  5009,  -408,  -408,   -81,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
     -41,   -18,    -3,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -126,  -408,  -408,   272,
     272,  -408,  -408,  -408,    72,   -74,   -36,   -34,   -22,    -9,
     -44,  -408,  4811,  -408,  -198,   -32,   -31,    36,  -162,  -408,
     111,  5329,  5355,  5592,  5355,  5355,  5355,  -408,   -16,  -408,
    5355,  -408,  -408,  -408,  -408,  -408,    74,  -408,    71,  5194,
     -25,  -408,  -408,  -408,  -408,  -408,  5355,  5355,  -408,  5355,
    -408,  -408,  -408,  5592,  -408,  -408,  -408,  -408,  -408,   -88,
    -408,  -408,  -408,   542,  -408,  -408,    89,    89,  -408,  -408,
    -408,    89,  -408,  5592,    89,    89,    89,    71,  -408,     6,
       8,  -207,    11,  -145,  -134,  -130,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  3751,     2,  -408,    -5,    66,    71,  1320,
    -408,  5194,    15,  -408,  -408,    14,  -119,  -408,  -408,    18,
      28,  2037,    53,    54,    34,  3294,    56,    57,  -408,  -408,
    -408,  -408,  -408,  -408,  4165,  4165,  4165,  -408,  -408,  -408,
    -408,  -408,    35,  -408,    59,  -408,  -101,  -408,  -408,  -408,
      60,   -96,  4372,    63,   -95,  4165,    13,   -77,   -70,  -115,
     103,    45,    52,    58,   151,   150,  -139,  -408,  -408,  -161,
    -408,    61,  5377,    78,  -408,  -408,  -408,  -408,   806,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,    71,
    -408,  -408,  -408,  -206,  3046,  -204,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,    76,  -408,  3958,  5194,  -408,   -16,  -150,
    -408,  -408,  -408,  1547,  -408,   129,  -408,   -88,  -408,  -408,
     173,  2550,  4165,  -408,  -408,  -149,  4165,  3544,  -408,  -408,
     -73,  -408,  2037,  -408,  -408,  4165,   111,  -408,  -408,  4165,
      80,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  4165,  -408,  4165,  4165,  4165,  4165,
    4165,  4165,  4165,  4165,  4165,  4165,  4165,  4165,  4165,  4165,
    4165,  4165,  4165,  4165,  4165,  4165,  4165,  -408,  -408,  -408,
     -16,  3046,  -203,  3046,  -408,  -408,  3046,  -408,  -408,    81,
      71,    64,  5194,     2,    71,  -408,  -408,  -408,  -408,  -408,
    -408,    83,  -408,  -408,  3544,   -59,  -408,   -53,    84,    71,
      85,  -408,  1070,    90,    84,  -408,    91,  -408,    87,   -50,
    4579,  -408,  -408,  -408,  -408,  -408,    13,    13,   -77,   -77,
     -70,   -70,   -70,   -70,  -115,  -115,   103,    45,    52,    58,
     151,   150,  -217,  -408,     2,  -408,  3046,  -408,  -159,  -408,
    -408,  -148,   182,  -408,  -408,  4165,  -408,    73,    94,  2037,
      77,    82,  2300,  -408,  -408,  -408,  -408,  -408,  4165,    95,
    -408,  4165,  -408,  2798,  -408,  -408,   -16,    79,   -35,  4165,
    2300,   328,  -408,   -14,  -408,  3046,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,     2,  -408,    93,    84,  -408,  2037,  4165,
      96,  -408,  -408,  1774,  2037,    -7,  -408,  -408,  -408,  -125,
    -408,  -408,  -408,  -408,  -408,  2037,  -408
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,    12,     0,     1,     2,    14,    15,    16,
       5,     0,     0,     0,    13,     6,     0,   183,   182,   207,
     203,   205,   206,   204,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   208,   209,   210,   220,   221,   222,   180,
     185,   186,   187,   184,   181,   163,   162,   161,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   244,   245,
     247,   248,   250,   251,   252,   253,   254,   256,   257,   258,
     259,   261,   262,   263,   264,   266,   267,   268,   270,   271,
     272,   273,   275,   243,   260,   269,   249,   255,   265,   274,
     276,   277,   278,   279,   280,   281,   246,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     188,   189,   190,   191,   192,   315,     0,   202,   201,   164,
     165,   318,   317,   316,     0,     0,     0,     0,     0,     0,
       0,   385,     3,   384,     0,     0,   119,   127,     0,   137,
     142,   169,   168,     0,   166,   167,   170,   149,   197,   199,
     171,   200,    18,   383,   116,   388,     0,   386,     0,     0,
       0,   185,   186,   187,    21,    22,   164,   165,   147,   169,
     173,   148,   172,     0,     7,     8,     9,    10,    11,     0,
      20,    19,   113,     0,   387,   117,   127,   127,   133,   134,
     135,   127,   120,     0,   127,   127,   127,     0,   114,    14,
      16,   143,     0,   185,   186,   187,   175,   389,   174,   150,
     176,   177,   179,     0,   198,   178,     0,     0,     0,     0,
     321,     0,     0,   160,   159,   156,     0,   152,   158,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    26,    27,
      24,    25,    28,    49,     0,     0,     0,    61,    62,    63,
      64,   351,   343,   347,    23,    30,    57,    32,    37,    38,
       0,     0,    43,     0,    65,     0,    69,    72,    75,    80,
      83,    85,    87,    89,    91,    93,    95,    97,   110,     0,
     333,     0,   169,   149,   336,   349,   335,   334,     0,   337,
     338,   339,   340,   341,   121,   128,   129,   125,   126,   136,
     130,   132,   131,   138,     0,   144,   122,   401,   402,   400,
     193,    65,   112,     0,    47,     0,     0,    17,   326,     0,
     324,   320,   322,     0,   115,     0,   151,     0,   379,   378,
       0,     0,     0,   382,   380,     0,     0,     0,    58,    59,
       0,   342,     0,    34,    35,     0,     0,    41,    40,     0,
     202,    44,    46,   100,   101,   103,   102,   105,   106,   107,
     108,   109,   104,    99,     0,    60,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   352,   348,   350,
     123,     0,   139,     0,   328,   146,     0,   194,   195,     0,
       0,     0,   397,   327,     0,   323,   319,   154,   155,   157,
     153,     0,   373,   372,   375,     0,   381,     0,   356,     0,
       0,    29,     0,     0,    36,    33,     0,    39,     0,     0,
      53,    45,    98,    66,    67,    68,    70,    71,    73,    74,
      78,    79,    76,    77,    81,    82,    84,    86,    88,    90,
      92,    94,     0,   111,   124,   141,     0,   331,     0,   145,
     196,     0,   394,   398,   325,     0,   374,     0,     0,     0,
       0,     0,     0,   344,    31,    56,    51,    50,     0,   202,
      54,     0,   140,     0,   329,   399,   395,     0,     0,   376,
       0,   355,   353,     0,   358,     0,   346,   369,   345,    55,
      96,   330,   332,   396,   390,     0,   377,   371,     0,     0,
       0,   359,   363,     0,   367,     0,   357,   370,   354,     0,
     362,   365,   364,   366,   360,   368,   361
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -408,  -408,  -408,  -408,  -408,  -408,    -1,   184,  -408,  -135,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  5182,  -408,  -112,  -111,  -160,  -114,
     -65,   -63,   -60,   -57,   -64,   -61,  -408,  -214,  -283,  -408,
    -258,     7,     3,     4,  -408,  -408,  -408,  -408,   144,   -10,
    -408,  -408,  -408,  -408,  -179,   -11,  -408,  -408,     5,  -408,
    -408,  -128,  -408,  -408,   -20,  -227,   -13,  -408,  -408,   -62,
    -408,   112,  -221,   -66,   -69,  -389,  -408,     9,  -260,  -407,
    -408,  -408,  -154,   203,    -4,    10,  -408,  -408,   -75,  -408,
    -408,  -171,  -408,  -172,  -408,  -408,  -408,  -408,  -408,  -408,
     212,  -408,  -408,  -151,  -408,  -408,   -56,  -408,  -408
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,    13,     3,   161,     6,   348,    14,   162,   284,
     285,   286,   453,   287,   288,   289,   290,   291,   292,   293,
     457,   458,   459,   460,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   394,
     309,   343,   310,   311,   165,   166,   167,   327,   222,   223,
     224,   328,   168,   169,   170,   199,   256,   439,   257,   258,
     172,   173,   174,   175,   176,   244,   344,   178,   179,   180,
     181,   249,   250,   349,   350,   425,   488,   314,   315,   316,
     317,   372,   527,   528,   318,   319,   320,   522,   450,   321,
     524,   542,   543,   544,   545,   322,   444,   497,   498,   323,
     182,   183,   184,   185,   186,   517,   431,   432,   187
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     177,   360,   171,    11,   335,   539,   540,   365,  -391,   381,
     248,    16,   539,   540,   198,   201,   163,   164,   370,  -392,
     237,   200,   202,  -393,     1,     7,     8,     9,   352,   342,
     414,   416,   485,     4,   487,   232,    12,   489,   243,   243,
     217,   345,   345,   236,   238,   511,   240,   241,   242,   403,
     404,   424,   245,     7,     8,     9,   334,   421,   419,   426,
     486,   373,   374,     7,     8,     9,   212,   213,   200,   202,
     248,   236,   248,   218,   219,   220,   383,   384,   385,   386,
     387,   388,   389,   390,   391,   392,   227,   416,     5,   513,
     401,   402,   203,   217,    10,   526,   461,   512,   434,   416,
     434,    15,   228,   417,   445,   226,   422,   514,   447,   448,
     188,   462,  -391,   526,   435,   446,   515,   454,   204,   337,
     253,   433,   415,   416,   532,   356,   218,   219,   220,   357,
     338,   342,   352,   483,   339,  -392,   546,   556,   424,   189,
     424,   405,   406,   424,   375,   190,   376,   225,   378,   177,
    -393,   171,   379,   254,   226,   226,   205,   482,   206,   226,
     239,   237,   226,   226,   226,   163,   164,   430,   393,   231,
     207,   451,   399,   400,   248,   416,   177,   510,   140,   141,
     142,   143,   144,   208,   236,   499,   448,   247,   449,   416,
     252,   500,   419,   484,   507,   416,   225,   225,   508,   209,
     313,   225,   312,   424,   225,   225,   225,   325,   255,   535,
     329,   326,   215,   416,   330,   331,   332,   216,   221,   151,
     152,   153,     7,     8,     9,   529,   333,   246,   530,   243,
     424,   140,   141,   142,   143,   144,   177,   518,   177,   521,
     251,   456,   424,   470,   471,   472,   473,   345,   313,   -21,
     312,   -22,   541,   430,   336,   151,   152,   153,   347,   554,
     346,   536,   229,     8,   230,   449,   396,   397,   398,   407,
     408,   221,   151,   152,   153,    17,    18,   355,   548,   354,
    -118,   549,   358,   551,   553,   437,   438,   466,   467,   533,
     468,   469,   359,   474,   475,   553,   361,   362,   363,   366,
     367,   371,   -48,   409,   377,   313,   382,   312,    39,   191,
     192,   410,   193,    43,    44,    45,    46,    47,   411,   412,
     413,   -47,   427,   441,   -42,   212,   495,   490,   420,   502,
     492,   506,   416,   177,   505,   516,   504,   519,   520,   -52,
     177,   538,   523,   534,   476,   525,   210,   477,   313,   480,
     312,   478,   429,   481,   313,   479,   255,   547,   550,   313,
     324,   312,   440,   353,   491,   494,   537,   214,   452,   496,
     442,   443,   552,   555,   211,   455,   493,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   140,   141,   142,   143,   144,   177,
       0,     0,     0,   194,     0,   195,     0,     0,     0,     0,
       0,   313,     0,     0,     0,     0,     0,     0,     0,   313,
       0,   312,     0,     0,     0,     0,     0,     0,   501,     0,
       0,     0,     0,   196,   197,   151,   152,   153,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   160,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   313,     0,   312,   313,
       0,   312,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   313,     0,   312,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   313,     0,   312,     0,     0,
     313,   313,   312,   312,     0,     0,     0,     0,     0,     0,
       0,     0,   313,     0,   312,    17,    18,    19,    20,    21,
      22,    23,   259,   260,   261,     0,   262,   263,   264,   265,
     266,     0,     0,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     0,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
       0,     0,     0,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   267,   194,   148,   195,   268,   269,   270,   271,
     272,   273,     0,     0,   274,   275,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   149,   150,   151,   152,   153,     0,   154,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   160,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   276,     0,     0,     0,     0,
       0,   277,   278,   279,   280,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   281,   282,   283,    17,
      18,    19,    20,    21,    22,    23,   259,   260,   261,     0,
     262,   263,   264,   265,   266,     0,     0,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     0,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,     0,     0,     0,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   267,   194,   148,   195,
     268,   269,   270,   271,   272,   273,     0,     0,   274,   275,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   149,   150,   151,
     152,   153,     0,   154,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   160,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   276,
       0,     0,     0,     0,     0,   277,   278,   279,   280,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     281,   282,   418,    17,    18,    19,    20,    21,    22,    23,
     259,   260,   261,     0,   262,   263,   264,   265,   266,     0,
       0,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     0,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,     0,     0,
       0,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     267,   194,   148,   195,   268,   269,   270,   271,   272,   273,
       0,     0,   274,   275,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   149,   150,   151,   152,   153,     0,   154,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     160,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   276,     0,     0,     0,     0,     0,   277,
     278,   279,   280,    17,    18,    19,    20,    21,    22,    23,
       0,     0,     0,     0,   281,   282,   503,     0,     0,     0,
       0,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,   191,   192,     0,
     193,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,     0,     0,
       0,     0,   140,   141,   142,   143,   144,   145,   146,   147,
       0,     0,   148,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   196,   197,   151,   152,   153,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     160,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      17,    18,    19,    20,    21,    22,    23,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,   191,   192,   351,   193,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,     0,     0,     0,     0,   140,
     141,   142,   143,   144,   145,   146,   147,     0,     0,   148,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   196,   197,
     151,   152,   153,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   160,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    17,    18,    19,
      20,    21,    22,    23,   259,   260,   261,     0,   262,   263,
     264,   265,   266,   539,   540,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,   436,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,     0,     0,     0,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   267,   194,   148,   195,   268,   269,
     270,   271,   272,   273,     0,     0,   274,   275,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   149,   150,   151,   152,   153,
       0,   154,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   160,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   276,     0,     0,
       0,     0,     0,   277,   278,   279,   280,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   281,   282,
      17,    18,    19,    20,    21,    22,    23,   259,   260,   261,
       0,   262,   263,   264,   265,   266,     0,     0,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     0,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,     0,     0,     0,     0,   140,
     141,   142,   143,   144,   145,   146,   147,   267,   194,   148,
     195,   268,   269,   270,   271,   272,   273,     0,     0,   274,
     275,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   149,   150,
     151,   152,   153,     0,   154,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   160,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     276,     0,     0,     0,     0,     0,   277,   278,   279,   280,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   281,   282,    17,    18,    19,    20,    21,    22,    23,
     259,   260,   261,     0,   262,   263,   264,   265,   266,     0,
       0,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     0,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,     0,     0,
       0,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     267,   194,   148,   195,   268,   269,   270,   271,   272,   273,
       0,     0,   274,   275,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   149,   150,   151,   152,   153,     0,   154,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     160,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   276,     0,     0,     0,     0,     0,   277,
     278,   279,   280,    17,    18,    19,    20,    21,    22,    23,
       0,     0,     0,     0,   281,   213,     0,     0,     0,     0,
       0,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     0,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,     0,     0,
       0,     0,   140,   141,   142,   143,   144,   145,   146,   147,
       0,   194,   148,   195,   268,   269,   270,   271,   272,   273,
       0,     0,   274,   275,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   149,   150,   151,   152,   153,     0,   154,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     160,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   276,     0,     0,     0,     0,     0,   277,
     278,   279,   280,    19,    20,    21,    22,    23,     0,     0,
       0,     0,     0,     0,   281,     0,     0,     0,     0,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   145,   146,   147,     0,   194,
     148,   195,   268,   269,   270,   271,   272,   273,     0,     0,
     274,   275,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   276,     0,     0,     0,     0,     0,   277,   278,   279,
     280,    19,    20,    21,    22,    23,     0,     0,     0,     0,
       0,     0,     0,   423,   531,     0,     0,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   145,   146,   147,     0,   194,   148,   195,
     268,   269,   270,   271,   272,   273,     0,     0,   274,   275,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   276,
       0,     0,     0,     0,     0,   277,   278,   279,   280,    19,
      20,    21,    22,    23,     0,     0,     0,     0,     0,     0,
       0,   423,     0,     0,     0,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   145,   146,   147,     0,   194,   148,   195,   268,   269,
     270,   271,   272,   273,     0,     0,   274,   275,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   276,     0,     0,
       0,     0,     0,   277,   278,   279,   280,    17,    18,    19,
      20,    21,    22,    23,     0,     0,     0,     0,   364,     0,
       0,     0,     0,     0,     0,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,   191,   192,     0,   193,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,     0,     0,     0,     0,   140,   141,   142,   143,
     144,   145,   146,   147,     0,   194,   148,   195,   268,   269,
     270,   271,   272,   273,     0,     0,   274,   275,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   196,   197,   151,   152,   153,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   160,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,    20,    21,    22,
      23,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,   276,     0,     0,
       0,     0,     0,   277,   278,   279,   280,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   145,   146,
     147,     0,   194,   148,   195,   268,   269,   270,   271,   272,
     273,     0,     0,   274,   275,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,    20,    21,    22,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,   276,     0,     0,   340,     0,     0,
     277,   278,   279,   280,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   145,   146,   147,     0,   194,
     148,   195,   268,   269,   270,   271,   272,   273,     0,     0,
     274,   275,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,    20,    21,    22,    23,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,   276,     0,     0,   428,     0,     0,   277,   278,   279,
     280,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   145,   146,   147,     0,   194,   148,   195,   268,
     269,   270,   271,   272,   273,     0,     0,   274,   275,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,    20,    21,
      22,    23,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,   276,     0,
       0,     0,     0,     0,   277,   278,   279,   280,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   145,
     146,   380,     0,   194,   148,   195,   268,   269,   270,   271,
     272,   273,     0,     0,   274,   275,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,    20,    21,    22,    23,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,   276,     0,     0,     0,     0,
       0,   277,   278,   279,   280,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   145,   146,   509,     0,
     194,   148,   195,   268,   269,   270,   271,   272,   273,     0,
       0,   274,   275,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,    18,    19,    20,    21,    22,
      23,     0,   276,     0,     0,     0,     0,     0,   277,   278,
     279,   280,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       0,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,     0,
       0,     0,     0,   140,   141,   142,   143,   144,   145,   146,
     147,     0,     0,   148,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   149,   150,   151,   152,   153,     0,   154,     0,
      12,     0,     0,     0,     0,     0,   155,   156,   157,   158,
     159,   160,    17,    18,    19,    20,    21,    22,    23,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     0,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,     0,     0,     0,
       0,   140,   141,   142,   143,   144,   145,   146,   147,     0,
       0,   148,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     149,   150,   151,   152,   153,     0,   154,    17,    18,    19,
      20,    21,    22,    23,   155,   156,   157,   158,   159,   160,
       0,     0,     0,     0,     0,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,   191,   192,     0,   193,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,    17,    18,     0,     0,   140,   141,   142,   143,
     144,   145,   146,   147,     0,     0,   148,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    17,    18,
       0,     0,     0,     0,     0,    39,   233,   234,     0,   235,
      43,    44,    45,    46,    47,   196,   197,   151,   152,   153,
      17,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    39,   191,   192,   160,   193,    43,    44,    45,    46,
      47,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    39,    40,    41,     0,    42,    43,    44,
      45,    46,    47,     0,     0,   341,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   368,   369,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   140,   141,   142,   143,   144,     0,   395,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   140,   141,   142,
     143,   144,     0,     0,     0,     0,     0,     0,     0,     0,
     196,   197,   151,   152,   153,     0,     0,     0,     0,   140,
     141,   142,   143,   144,     0,     0,     0,   341,     0,   160,
       0,     0,     0,     0,     0,     0,   196,   197,   151,   152,
     153,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   160,     0,     0,   196,   197,
     151,   152,   153,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   160,   463,   464,
     465,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,    19,    20,    21,
      22,    23,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   145,
     146,   147,     0,     0,   148
};

static const yytype_int16 yycheck[] =
{
      13,   261,    13,     4,   231,    19,    20,   265,   153,   292,
     189,    12,    19,    20,   149,   150,    13,    13,   276,   153,
     171,   149,   150,   153,   188,   151,   152,   153,   249,   243,
     169,   248,   421,   156,   423,   170,   189,   426,   245,   245,
       4,   245,   245,   171,   172,   262,   174,   175,   176,   164,
     165,   334,   180,   151,   152,   153,   263,   263,   318,   263,
     263,   162,   163,   151,   152,   153,   264,   265,   196,   197,
     249,   199,   251,    37,    38,    39,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   248,   248,     0,   248,
     160,   161,   154,     4,   192,   502,   379,   486,   248,   248,
     248,   192,   264,   264,   362,   167,   333,   266,   366,   367,
     191,   394,   153,   520,   264,   264,   264,   375,   192,   264,
     208,   348,   261,   248,   513,   244,    37,    38,    39,   248,
     264,   345,   353,   416,   264,   153,   525,   262,   421,   265,
     423,   256,   257,   426,   245,   146,   247,   167,   244,   162,
     153,   162,   248,   241,   216,   217,   192,   415,   192,   221,
     173,   312,   224,   225,   226,   162,   162,   346,   263,   170,
     192,   244,   249,   250,   353,   248,   189,   460,   142,   143,
     144,   145,   146,   192,   312,   244,   444,   188,   367,   248,
     203,   244,   452,   420,   244,   248,   216,   217,   248,   243,
     213,   221,   213,   486,   224,   225,   226,   217,   209,   244,
     223,   221,   244,   248,   224,   225,   226,   248,   182,   183,
     184,   185,   151,   152,   153,   508,   227,   153,   511,   245,
     513,   142,   143,   144,   145,   146,   249,   495,   251,   499,
     265,   376,   525,   403,   404,   405,   406,   245,   261,   243,
     261,   243,   266,   432,   243,   183,   184,   185,   192,   266,
     265,   519,   151,   152,   153,   444,   253,   254,   255,   166,
     167,   182,   183,   184,   185,     3,     4,   263,   538,   264,
     244,   539,   264,   543,   544,   156,   157,   399,   400,   516,
     401,   402,   264,   407,   408,   555,   243,   243,   264,   243,
     243,   266,   243,   258,   244,   318,   243,   318,    36,    37,
      38,   259,    40,    41,    42,    43,    44,    45,   260,   168,
     170,   243,   246,   150,   244,   264,   243,   246,   329,   244,
     266,   244,   248,   346,   243,   153,   246,   264,   244,   244,
     353,    13,   265,   264,   409,   263,   162,   410,   361,   413,
     361,   411,   345,   414,   367,   412,   357,   264,   262,   372,
     216,   372,   357,   251,   430,   434,   520,   164,   372,   444,
     361,   361,   543,   545,   162,   376,   432,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   142,   143,   144,   145,   146,   432,
      -1,    -1,    -1,   151,    -1,   153,    -1,    -1,    -1,    -1,
      -1,   444,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   452,
      -1,   452,    -1,    -1,    -1,    -1,    -1,    -1,   449,    -1,
      -1,    -1,    -1,   181,   182,   183,   184,   185,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   499,    -1,   499,   502,
      -1,   502,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   520,    -1,   520,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   538,    -1,   538,    -1,    -1,
     543,   544,   543,   544,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   555,    -1,   555,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    -1,    14,    15,    16,    17,
      18,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,    -1,    -1,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   181,   182,   183,   184,   185,    -1,   187,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   243,    -1,    -1,    -1,    -1,
      -1,   249,   250,   251,   252,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   264,   265,   266,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    -1,
      14,    15,    16,    17,    18,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,    -1,    -1,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,   183,
     184,   185,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   243,
      -1,    -1,    -1,    -1,    -1,   249,   250,   251,   252,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     264,   265,   266,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    -1,    14,    15,    16,    17,    18,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
      -1,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,   182,   183,   184,   185,    -1,   187,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   243,    -1,    -1,    -1,    -1,    -1,   249,
     250,   251,   252,     3,     4,     5,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,   264,   265,   266,    -1,    -1,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,   148,   149,
      -1,    -1,   152,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,   182,   183,   184,   185,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,   266,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,   142,
     143,   144,   145,   146,   147,   148,   149,    -1,    -1,   152,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,
     183,   184,   185,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,   266,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    -1,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,    -1,    -1,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,
      -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   243,    -1,    -1,
      -1,    -1,    -1,   249,   250,   251,   252,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   264,   265,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      -1,    14,    15,    16,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,    -1,    -1,   162,
     163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,   182,
     183,   184,   185,    -1,   187,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     243,    -1,    -1,    -1,    -1,    -1,   249,   250,   251,   252,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   264,   265,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    -1,    14,    15,    16,    17,    18,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
      -1,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,   182,   183,   184,   185,    -1,   187,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   243,    -1,    -1,    -1,    -1,    -1,   249,
     250,   251,   252,     3,     4,     5,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,   264,   265,    -1,    -1,    -1,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,   148,   149,
      -1,   151,   152,   153,   154,   155,   156,   157,   158,   159,
      -1,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,   182,   183,   184,   185,    -1,   187,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   243,    -1,    -1,    -1,    -1,    -1,   249,
     250,   251,   252,     5,     6,     7,     8,     9,    -1,    -1,
      -1,    -1,    -1,    -1,   264,    -1,    -1,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   147,   148,   149,    -1,   151,
     152,   153,   154,   155,   156,   157,   158,   159,    -1,    -1,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   243,    -1,    -1,    -1,    -1,    -1,   249,   250,   251,
     252,     5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   265,   266,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   147,   148,   149,    -1,   151,   152,   153,
     154,   155,   156,   157,   158,   159,    -1,    -1,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   243,
      -1,    -1,    -1,    -1,    -1,   249,   250,   251,   252,     5,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   265,    -1,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   147,   148,   149,    -1,   151,   152,   153,   154,   155,
     156,   157,   158,   159,    -1,    -1,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   243,    -1,    -1,
      -1,    -1,    -1,   249,   250,   251,   252,     3,     4,     5,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,   264,    -1,
      -1,    -1,    -1,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,    -1,    -1,    -1,    -1,   142,   143,   144,   145,
     146,   147,   148,   149,    -1,   151,   152,   153,   154,   155,
     156,   157,   158,   159,    -1,    -1,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     5,     6,     7,     8,
       9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,   243,    -1,    -1,
      -1,    -1,    -1,   249,   250,   251,   252,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   147,   148,
     149,    -1,   151,   152,   153,   154,   155,   156,   157,   158,
     159,    -1,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     5,     6,     7,     8,     9,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,   243,    -1,    -1,   246,    -1,    -1,
     249,   250,   251,   252,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   147,   148,   149,    -1,   151,
     152,   153,   154,   155,   156,   157,   158,   159,    -1,    -1,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,   243,    -1,    -1,   246,    -1,    -1,   249,   250,   251,
     252,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   147,   148,   149,    -1,   151,   152,   153,   154,
     155,   156,   157,   158,   159,    -1,    -1,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,   243,    -1,
      -1,    -1,    -1,    -1,   249,   250,   251,   252,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   147,
     148,   149,    -1,   151,   152,   153,   154,   155,   156,   157,
     158,   159,    -1,    -1,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     5,     6,     7,     8,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,   243,    -1,    -1,    -1,    -1,
      -1,   249,   250,   251,   252,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   147,   148,   149,    -1,
     151,   152,   153,   154,   155,   156,   157,   158,   159,    -1,
      -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,     8,
       9,    -1,   243,    -1,    -1,    -1,    -1,    -1,   249,   250,
     251,   252,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,    -1,
      -1,    -1,    -1,   142,   143,   144,   145,   146,   147,   148,
     149,    -1,    -1,   152,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,   182,   183,   184,   185,    -1,   187,    -1,
     189,    -1,    -1,    -1,    -1,    -1,   195,   196,   197,   198,
     199,   200,     3,     4,     5,     6,     7,     8,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,    -1,    -1,    -1,
      -1,   142,   143,   144,   145,   146,   147,   148,   149,    -1,
      -1,   152,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,   182,   183,   184,   185,    -1,   187,     3,     4,     5,
       6,     7,     8,     9,   195,   196,   197,   198,   199,   200,
      -1,    -1,    -1,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,     3,     4,    -1,    -1,   142,   143,   144,   145,
     146,   147,   148,   149,    -1,    -1,   152,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,    -1,    -1,    -1,    -1,    36,    37,    38,    -1,    40,
      41,    42,    43,    44,    45,   181,   182,   183,   184,   185,
       3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    37,    38,   200,    40,    41,    42,    43,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    37,    38,    -1,    40,    41,    42,
      43,    44,    45,    -1,    -1,   243,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   274,   275,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   142,   143,   144,   145,   146,    -1,   295,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,   182,   183,   184,   185,    -1,    -1,    -1,    -1,   142,
     143,   144,   145,   146,    -1,    -1,    -1,   345,    -1,   200,
      -1,    -1,    -1,    -1,    -1,    -1,   181,   182,   183,   184,
     185,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,   181,   182,
     183,   184,   185,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,   396,   397,
     398,   399,   400,   401,   402,   403,   404,   405,   406,   407,
     408,   409,   410,   411,   412,   413,   414,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   147,
     148,   149,    -1,    -1,   152
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   188,   268,   270,   156,     0,   272,   151,   152,   153,
     192,   273,   189,   269,   274,   192,   273,     3,     4,     5,
       6,     7,     8,     9,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     142,   143,   144,   145,   146,   147,   148,   149,   152,   181,
     182,   183,   184,   185,   187,   195,   196,   197,   198,   199,
     200,   271,   275,   309,   310,   311,   312,   313,   319,   320,
     321,   322,   327,   328,   329,   330,   331,   333,   334,   335,
     336,   337,   367,   368,   369,   370,   371,   375,   191,   265,
     273,    37,    38,    40,   151,   153,   181,   182,   276,   322,
     328,   276,   328,   336,   192,   192,   192,   192,   192,   243,
     274,   367,   264,   265,   350,   244,   248,     4,    37,    38,
      39,   182,   315,   316,   317,   331,   336,   248,   264,   151,
     153,   273,   276,    37,    38,    40,   328,   370,   328,   333,
     328,   328,   328,   245,   332,   328,   153,   273,   321,   338,
     339,   265,   333,   208,   241,   273,   323,   325,   326,    10,
      11,    12,    14,    15,    16,    17,    18,   150,   154,   155,
     156,   157,   158,   159,   162,   163,   243,   249,   250,   251,
     252,   264,   265,   266,   276,   277,   278,   280,   281,   282,
     283,   284,   285,   286,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   307,
     309,   310,   322,   333,   344,   345,   346,   347,   351,   352,
     353,   356,   362,   366,   315,   316,   316,   314,   318,   333,
     316,   316,   316,   273,   263,   332,   243,   264,   264,   264,
     246,   291,   304,   308,   333,   245,   265,   192,   273,   340,
     341,   266,   339,   338,   264,   263,   244,   248,   264,   264,
     345,   243,   243,   264,   264,   307,   243,   243,   291,   291,
     307,   266,   348,   162,   163,   245,   247,   244,   244,   248,
     149,   305,   243,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   263,   306,   291,   253,   254,   255,   249,
     250,   160,   161,   164,   165,   256,   257,   166,   167,   258,
     259,   260,   168,   170,   169,   261,   248,   264,   266,   345,
     273,   263,   332,   265,   305,   342,   263,   246,   246,   308,
     321,   373,   374,   332,   248,   264,   266,   156,   157,   324,
     325,   150,   344,   352,   363,   307,   264,   307,   307,   321,
     355,   244,   351,   279,   307,   273,   276,   287,   288,   289,
     290,   305,   305,   291,   291,   291,   293,   293,   294,   294,
     295,   295,   295,   295,   296,   296,   297,   298,   299,   300,
     301,   302,   307,   305,   332,   342,   263,   342,   343,   342,
     246,   340,   266,   373,   341,   243,   355,   364,   365,   244,
     244,   273,   244,   266,   246,   243,   244,   244,   248,   149,
     305,   262,   342,   248,   266,   264,   153,   372,   307,   264,
     244,   345,   354,   265,   357,   263,   346,   349,   350,   305,
     305,   266,   342,   332,   264,   244,   307,   349,    13,    19,
      20,   266,   358,   359,   360,   361,   342,   264,   345,   307,
     262,   345,   358,   345,   266,   360,   262
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   267,   269,   268,   270,   270,   270,   271,   271,   271,
     271,   271,   272,   272,   273,   273,   273,   274,   275,   275,
     275,   276,   276,   277,   277,   277,   277,   277,   277,   277,
     278,   278,   278,   278,   278,   278,   279,   280,   281,   281,
     282,   282,   283,   283,   284,   284,   285,   286,   286,   286,
     287,   287,   288,   288,   289,   289,   290,   291,   291,   291,
     291,   292,   292,   292,   292,   293,   293,   293,   293,   294,
     294,   294,   295,   295,   295,   296,   296,   296,   296,   296,
     297,   297,   297,   298,   298,   299,   299,   300,   300,   301,
     301,   302,   302,   303,   303,   304,   304,   305,   305,   306,
     306,   306,   306,   306,   306,   306,   306,   306,   306,   306,
     307,   307,   308,   309,   309,   309,   309,   310,   311,   311,
     312,   312,   313,   314,   314,   315,   315,   316,   316,   316,
     316,   316,   316,   317,   317,   317,   318,   319,   319,   319,
     319,   319,   320,   320,   320,   320,   320,   320,   320,   321,
     321,   322,   323,   323,   324,   324,   325,   325,   325,   326,
     326,   327,   327,   327,   328,   328,   328,   328,   328,   328,
     328,   328,   328,   328,   328,   328,   328,   328,   328,   328,
     329,   329,   330,   330,   330,   330,   330,   330,   331,   331,
     331,   331,   331,   332,   332,   332,   332,   333,   333,   334,
     334,   334,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   336,   336,   336,   337,
     337,   338,   338,   339,   340,   340,   341,   341,   342,   342,
     342,   343,   343,   344,   345,   345,   346,   346,   346,   346,
     346,   346,   347,   348,   347,   349,   349,   350,   350,   351,
     351,   352,   352,   353,   354,   354,   355,   355,   356,   357,
     357,   358,   358,   359,   359,   360,   360,   361,   361,   362,
     362,   362,   363,   363,   364,   364,   365,   365,   366,   366,
     366,   366,   366,   367,   367,   367,   367,   368,   369,   369,
     370,   371,   371,   371,   372,   372,   372,   373,   373,   374,
     375,   375,   375
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     4,     0,     3,     4,     2,     2,     2,
       2,     2,     0,     2,     1,     1,     1,     5,     1,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     4,     1,     3,     2,     2,     1,     1,     1,     3,
       2,     2,     2,     1,     2,     3,     2,     1,     1,     1,
       2,     2,     2,     1,     2,     3,     2,     1,     2,     2,
       2,     1,     1,     1,     1,     1,     3,     3,     3,     1,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     3,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     5,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     2,     2,     4,     1,     2,     1,     1,
       2,     3,     3,     2,     3,     2,     2,     0,     2,     2,
       2,     2,     2,     1,     1,     1,     1,     1,     3,     4,
       6,     5,     1,     2,     3,     5,     4,     2,     2,     1,
       2,     4,     1,     3,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     3,     3,     4,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     5,
       4,     1,     2,     3,     1,     3,     1,     2,     1,     3,
       4,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     0,     4,     1,     1,     2,     3,     1,
       2,     1,     2,     5,     3,     1,     1,     4,     5,     2,
       3,     3,     2,     1,     2,     2,     2,     1,     2,     5,
       7,     6,     1,     1,     1,     0,     2,     3,     2,     2,
       2,     3,     2,     1,     1,     1,     1,     2,     1,     2,
       7,     1,     1,     1,     0,     1,     2,     1,     2,     3,
       3,     3,     3
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct _mesa_glsl_parse_state *state)
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct _mesa_glsl_parse_state *state)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, struct _mesa_glsl_parse_state *state)
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, struct _mesa_glsl_parse_state *state)
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
yyparse (struct _mesa_glsl_parse_state *state)
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

/* User initialization code.  */
#line 87 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1429  */
{
   yylloc.first_line = 1;
   yylloc.first_column = 1;
   yylloc.last_line = 1;
   yylloc.last_column = 1;
   yylloc.source = 0;
}

#line 3084 "glsl_parser.cpp" /* yacc.c:1429  */
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
        case 2:
#line 308 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      _mesa_glsl_initialize_types(state);
   }
#line 3275 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 312 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      delete state->symbols;
      state->symbols = new(ralloc_parent(state)) glsl_symbol_table;
      _mesa_glsl_initialize_types(state);
   }
#line 3285 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 322 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      state->process_version_directive(&(yylsp[-1]), (yyvsp[-1].n), NULL);
      if (state->error) {
         YYERROR;
      }
   }
#line 3296 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 329 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      state->process_version_directive(&(yylsp[-2]), (yyvsp[-2].n), (yyvsp[-1].identifier));
      if (state->error) {
         YYERROR;
      }
   }
#line 3307 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 343 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      /* Pragma invariant(all) cannot be used in a fragment shader.
       *
       * Page 27 of the GLSL 1.20 spec, Page 53 of the GLSL ES 3.00 spec:
       *
       *     "It is an error to use this pragma in a fragment shader."
       */
      if (state->is_version(120, 300) &&
          state->stage == MESA_SHADER_FRAGMENT) {
         _mesa_glsl_error(& (yylsp[-1]), state,
                          "pragma `invariant(all)' cannot be used "
                          "in a fragment shader.");
      } else if (!state->is_version(120, 100)) {
         _mesa_glsl_warning(& (yylsp[-1]), state,
                            "pragma `invariant(all)' not supported in %s "
                            "(GLSL ES 1.00 or GLSL 1.20 required)",
                            state->get_version_string());
      } else {
         state->all_invariant = true;
      }
   }
#line 3333 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 379 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if (!_mesa_glsl_process_extension((yyvsp[-3].identifier), & (yylsp[-3]), (yyvsp[-1].identifier), & (yylsp[-1]), state)) {
         YYERROR;
      }
   }
#line 3343 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 388 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      /* FINISHME: The NULL test is required because pragmas are set to
       * FINISHME: NULL. (See production rule for external_declaration.)
       */
      if ((yyvsp[0].node) != NULL)
         state->translation_unit.push_tail(& (yyvsp[0].node)->link);
   }
#line 3355 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 396 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      /* FINISHME: The NULL test is required because pragmas are set to
       * FINISHME: NULL. (See production rule for external_declaration.)
       */
      if ((yyvsp[0].node) != NULL)
         state->translation_unit.push_tail(& (yyvsp[0].node)->link);
   }
#line 3367 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 403 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if (!state->allow_extension_directive_midshader) {
         _mesa_glsl_error(& (yylsp[0]), state,
                          "#extension directive is not allowed "
                          "in the middle of a shader");
         YYERROR;
      }
   }
#line 3380 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 420 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_identifier, NULL, NULL, NULL);
      (yyval.expression)->set_location((yylsp[0]));
      (yyval.expression)->primary_expression.identifier = (yyvsp[0].identifier);
   }
#line 3391 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 427 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_int_constant, NULL, NULL, NULL);
      (yyval.expression)->set_location((yylsp[0]));
      (yyval.expression)->primary_expression.int_constant = (yyvsp[0].n);
   }
#line 3402 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 434 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_uint_constant, NULL, NULL, NULL);
      (yyval.expression)->set_location((yylsp[0]));
      (yyval.expression)->primary_expression.uint_constant = (yyvsp[0].n);
   }
#line 3413 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 441 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_float_constant, NULL, NULL, NULL);
      (yyval.expression)->set_location((yylsp[0]));
      (yyval.expression)->primary_expression.float_constant = (yyvsp[0].real);
   }
#line 3424 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 448 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_double_constant, NULL, NULL, NULL);
      (yyval.expression)->set_location((yylsp[0]));
      (yyval.expression)->primary_expression.double_constant = (yyvsp[0].dreal);
   }
#line 3435 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 455 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_bool_constant, NULL, NULL, NULL);
      (yyval.expression)->set_location((yylsp[0]));
      (yyval.expression)->primary_expression.bool_constant = (yyvsp[0].n);
   }
#line 3446 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 462 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.expression) = (yyvsp[-1].expression);
   }
#line 3454 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 470 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_array_index, (yyvsp[-3].expression), (yyvsp[-1].expression), NULL);
      (yyval.expression)->set_location_range((yylsp[-3]), (yylsp[0]));
   }
#line 3464 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 476 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.expression) = (yyvsp[0].expression);
   }
#line 3472 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 480 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_field_selection, (yyvsp[-2].expression), NULL, NULL);
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
      (yyval.expression)->primary_expression.identifier = (yyvsp[0].identifier);
   }
#line 3483 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 487 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_post_inc, (yyvsp[-1].expression), NULL, NULL);
      (yyval.expression)->set_location_range((yylsp[-1]), (yylsp[0]));
   }
#line 3493 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 493 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_post_dec, (yyvsp[-1].expression), NULL, NULL);
      (yyval.expression)->set_location_range((yylsp[-1]), (yylsp[0]));
   }
#line 3503 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 511 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_field_selection, (yyvsp[-2].expression), (yyvsp[0].expression), NULL);
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3513 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 530 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.expression) = (yyvsp[-1].expression);
      (yyval.expression)->set_location((yylsp[-1]));
      (yyval.expression)->expressions.push_tail(& (yyvsp[0].expression)->link);
   }
#line 3523 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 536 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.expression) = (yyvsp[-2].expression);
      (yyval.expression)->set_location((yylsp[-2]));
      (yyval.expression)->expressions.push_tail(& (yyvsp[0].expression)->link);
   }
#line 3533 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 552 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_function_expression((yyvsp[0].type_specifier));
      (yyval.expression)->set_location((yylsp[0]));
      }
#line 3543 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 558 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_expression *callee = new(ctx) ast_expression((yyvsp[0].identifier));
      callee->set_location((yylsp[0]));
      (yyval.expression) = new(ctx) ast_function_expression(callee);
      (yyval.expression)->set_location((yylsp[0]));
      }
#line 3555 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 566 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_expression *callee = new(ctx) ast_expression((yyvsp[0].identifier));
      callee->set_location((yylsp[0]));
      (yyval.expression) = new(ctx) ast_function_expression(callee);
      (yyval.expression)->set_location((yylsp[0]));
      }
#line 3567 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 587 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.expression) = (yyvsp[-1].expression);
      (yyval.expression)->set_location((yylsp[-1]));
      (yyval.expression)->expressions.push_tail(& (yyvsp[0].expression)->link);
   }
#line 3577 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 593 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.expression) = (yyvsp[-2].expression);
      (yyval.expression)->set_location((yylsp[-2]));
      (yyval.expression)->expressions.push_tail(& (yyvsp[0].expression)->link);
   }
#line 3587 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 605 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_expression *callee = new(ctx) ast_expression((yyvsp[-1].identifier));
      callee->set_location((yylsp[-1]));
      (yyval.expression) = new(ctx) ast_function_expression(callee);
      (yyval.expression)->set_location((yylsp[-1]));
   }
#line 3599 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 618 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_pre_inc, (yyvsp[0].expression), NULL, NULL);
      (yyval.expression)->set_location((yylsp[-1]));
   }
#line 3609 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 624 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_pre_dec, (yyvsp[0].expression), NULL, NULL);
      (yyval.expression)->set_location((yylsp[-1]));
   }
#line 3619 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 630 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression((yyvsp[-1].n), (yyvsp[0].expression), NULL, NULL);
      (yyval.expression)->set_location_range((yylsp[-1]), (yylsp[0]));
   }
#line 3629 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 639 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_plus; }
#line 3635 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 640 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_neg; }
#line 3641 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 641 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_logic_not; }
#line 3647 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 642 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_bit_not; }
#line 3653 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 648 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_mul, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3663 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 654 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_div, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3673 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 660 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_mod, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3683 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 670 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_add, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3693 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 676 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_sub, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3703 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 686 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_lshift, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3713 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 692 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_rshift, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3723 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 702 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_less, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3733 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 708 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_greater, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3743 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 714 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_lequal, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3753 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 720 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_gequal, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3763 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 730 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_equal, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3773 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 736 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_nequal, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3783 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 746 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_bit_and, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3793 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 756 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_bit_xor, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3803 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 766 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_bit_or, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3813 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 776 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_logic_and, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3823 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 786 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_logic_xor, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3833 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 796 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression_bin(ast_logic_or, (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3843 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 806 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression(ast_conditional, (yyvsp[-4].expression), (yyvsp[-2].expression), (yyvsp[0].expression));
      (yyval.expression)->set_location_range((yylsp[-4]), (yylsp[0]));
   }
#line 3853 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 816 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_expression((yyvsp[-1].n), (yyvsp[-2].expression), (yyvsp[0].expression), NULL);
      (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 3863 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 824 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_assign; }
#line 3869 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 825 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_mul_assign; }
#line 3875 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 826 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_div_assign; }
#line 3881 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 827 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_mod_assign; }
#line 3887 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 828 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_add_assign; }
#line 3893 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 829 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_sub_assign; }
#line 3899 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 830 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_ls_assign; }
#line 3905 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 831 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_rs_assign; }
#line 3911 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 832 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_and_assign; }
#line 3917 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 833 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_xor_assign; }
#line 3923 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 834 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = ast_or_assign; }
#line 3929 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 839 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.expression) = (yyvsp[0].expression);
   }
#line 3937 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 843 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      if ((yyvsp[-2].expression)->oper != ast_sequence) {
         (yyval.expression) = new(ctx) ast_expression(ast_sequence, NULL, NULL, NULL);
         (yyval.expression)->set_location_range((yylsp[-2]), (yylsp[0]));
         (yyval.expression)->expressions.push_tail(& (yyvsp[-2].expression)->link);
      } else {
         (yyval.expression) = (yyvsp[-2].expression);
      }

      (yyval.expression)->expressions.push_tail(& (yyvsp[0].expression)->link);
   }
#line 3954 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 863 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      state->symbols->pop_scope();
      (yyval.node) = (yyvsp[-1].function);
   }
#line 3963 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 868 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.node) = (yyvsp[-1].declarator_list);
   }
#line 3971 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 872 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyvsp[-1].type_specifier)->default_precision = (yyvsp[-2].n);
      (yyval.node) = (yyvsp[-1].type_specifier);
   }
#line 3980 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 877 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.node) = (yyvsp[0].node);
   }
#line 3988 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 893 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.function) = (yyvsp[-1].function);
      (yyval.function)->parameters.push_tail(& (yyvsp[0].parameter_declarator)->link);
   }
#line 3997 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 898 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.function) = (yyvsp[-2].function);
      (yyval.function)->parameters.push_tail(& (yyvsp[0].parameter_declarator)->link);
   }
#line 4006 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 906 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.function) = new(ctx) ast_function();
      (yyval.function)->set_location((yylsp[-1]));
      (yyval.function)->return_type = (yyvsp[-2].fully_specified_type);
      (yyval.function)->identifier = (yyvsp[-1].identifier);

      state->symbols->add_function(new(state) ir_function((yyvsp[-1].identifier)));
      state->symbols->push_scope();
   }
#line 4021 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 920 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.parameter_declarator) = new(ctx) ast_parameter_declarator();
      (yyval.parameter_declarator)->set_location_range((yylsp[-1]), (yylsp[0]));
      (yyval.parameter_declarator)->type = new(ctx) ast_fully_specified_type();
      (yyval.parameter_declarator)->type->set_location((yylsp[-1]));
      (yyval.parameter_declarator)->type->specifier = (yyvsp[-1].type_specifier);
      (yyval.parameter_declarator)->identifier = (yyvsp[0].identifier);
   }
#line 4035 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 930 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.parameter_declarator) = new(ctx) ast_parameter_declarator();
      (yyval.parameter_declarator)->set_location_range((yylsp[-2]), (yylsp[0]));
      (yyval.parameter_declarator)->type = new(ctx) ast_fully_specified_type();
      (yyval.parameter_declarator)->type->set_location((yylsp[-2]));
      (yyval.parameter_declarator)->type->specifier = (yyvsp[-2].type_specifier);
      (yyval.parameter_declarator)->identifier = (yyvsp[-1].identifier);
      (yyval.parameter_declarator)->array_specifier = (yyvsp[0].array_specifier);
   }
#line 4050 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 944 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.parameter_declarator) = (yyvsp[0].parameter_declarator);
      (yyval.parameter_declarator)->type->qualifier = (yyvsp[-1].type_qualifier);
   }
#line 4059 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 949 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.parameter_declarator) = new(ctx) ast_parameter_declarator();
      (yyval.parameter_declarator)->set_location((yylsp[0]));
      (yyval.parameter_declarator)->type = new(ctx) ast_fully_specified_type();
      (yyval.parameter_declarator)->type->set_location_range((yylsp[-1]), (yylsp[0]));
      (yyval.parameter_declarator)->type->qualifier = (yyvsp[-1].type_qualifier);
      (yyval.parameter_declarator)->type->specifier = (yyvsp[0].type_specifier);
   }
#line 4073 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 962 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
   }
#line 4081 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 966 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[0].type_qualifier).flags.q.constant)
         _mesa_glsl_error(&(yylsp[-1]), state, "duplicate const qualifier");

      (yyval.type_qualifier) = (yyvsp[0].type_qualifier);
      (yyval.type_qualifier).flags.q.constant = 1;
   }
#line 4093 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 974 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[0].type_qualifier).flags.q.precise)
         _mesa_glsl_error(&(yylsp[-1]), state, "duplicate precise qualifier");

      (yyval.type_qualifier) = (yyvsp[0].type_qualifier);
      (yyval.type_qualifier).flags.q.precise = 1;
   }
#line 4105 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 982 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if (((yyvsp[-1].type_qualifier).flags.q.in || (yyvsp[-1].type_qualifier).flags.q.out) && ((yyvsp[0].type_qualifier).flags.q.in || (yyvsp[0].type_qualifier).flags.q.out))
         _mesa_glsl_error(&(yylsp[-1]), state, "duplicate in/out/inout qualifier");

      if (!state->ARB_shading_language_420pack_enable && (yyvsp[0].type_qualifier).flags.q.constant)
         _mesa_glsl_error(&(yylsp[-1]), state, "in/out/inout must come after const "
                                      "or precise");

      (yyval.type_qualifier) = (yyvsp[-1].type_qualifier);
      (yyval.type_qualifier).merge_qualifier(&(yylsp[-1]), state, (yyvsp[0].type_qualifier));
   }
#line 4121 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 994 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[0].type_qualifier).precision != ast_precision_none)
         _mesa_glsl_error(&(yylsp[-1]), state, "duplicate precision qualifier");

      if (!state->ARB_shading_language_420pack_enable && (yyvsp[0].type_qualifier).flags.i != 0)
         _mesa_glsl_error(&(yylsp[-1]), state, "precision qualifiers must come last");

      (yyval.type_qualifier) = (yyvsp[0].type_qualifier);
      (yyval.type_qualifier).precision = (yyvsp[-1].n);
   }
#line 4136 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 1005 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.type_qualifier) = (yyvsp[-1].type_qualifier);
      (yyval.type_qualifier).merge_qualifier(&(yylsp[-1]), state, (yyvsp[0].type_qualifier));
   }
#line 4145 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1012 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.in = 1;
   }
#line 4154 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1017 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.out = 1;
   }
#line 4163 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1022 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.in = 1;
      (yyval.type_qualifier).flags.q.out = 1;
   }
#line 4173 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1036 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[0].identifier), NULL, NULL);
      decl->set_location((yylsp[0]));

      (yyval.declarator_list) = (yyvsp[-2].declarator_list);
      (yyval.declarator_list)->declarations.push_tail(&decl->link);
      state->symbols->add_variable(new(state) ir_variable(NULL, (yyvsp[0].identifier), ir_var_auto));
   }
#line 4187 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1046 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[-1].identifier), (yyvsp[0].array_specifier), NULL);
      decl->set_location_range((yylsp[-1]), (yylsp[0]));

      (yyval.declarator_list) = (yyvsp[-3].declarator_list);
      (yyval.declarator_list)->declarations.push_tail(&decl->link);
      state->symbols->add_variable(new(state) ir_variable(NULL, (yyvsp[-1].identifier), ir_var_auto));
   }
#line 4201 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1056 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[-3].identifier), (yyvsp[-2].array_specifier), (yyvsp[0].expression));
      decl->set_location_range((yylsp[-3]), (yylsp[-2]));

      (yyval.declarator_list) = (yyvsp[-5].declarator_list);
      (yyval.declarator_list)->declarations.push_tail(&decl->link);
      state->symbols->add_variable(new(state) ir_variable(NULL, (yyvsp[-3].identifier), ir_var_auto));
   }
#line 4215 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1066 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[-2].identifier), NULL, (yyvsp[0].expression));
      decl->set_location((yylsp[-2]));

      (yyval.declarator_list) = (yyvsp[-4].declarator_list);
      (yyval.declarator_list)->declarations.push_tail(&decl->link);
      state->symbols->add_variable(new(state) ir_variable(NULL, (yyvsp[-2].identifier), ir_var_auto));
   }
#line 4229 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1080 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      /* Empty declaration list is valid. */
      (yyval.declarator_list) = new(ctx) ast_declarator_list((yyvsp[0].fully_specified_type));
      (yyval.declarator_list)->set_location((yylsp[0]));
   }
#line 4240 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1087 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[0].identifier), NULL, NULL);
      decl->set_location((yylsp[0]));

      (yyval.declarator_list) = new(ctx) ast_declarator_list((yyvsp[-1].fully_specified_type));
      (yyval.declarator_list)->set_location_range((yylsp[-1]), (yylsp[0]));
      (yyval.declarator_list)->declarations.push_tail(&decl->link);
   }
#line 4254 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1097 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[-1].identifier), (yyvsp[0].array_specifier), NULL);
      decl->set_location_range((yylsp[-1]), (yylsp[0]));

      (yyval.declarator_list) = new(ctx) ast_declarator_list((yyvsp[-2].fully_specified_type));
      (yyval.declarator_list)->set_location_range((yylsp[-2]), (yylsp[0]));
      (yyval.declarator_list)->declarations.push_tail(&decl->link);
   }
#line 4268 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1107 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[-3].identifier), (yyvsp[-2].array_specifier), (yyvsp[0].expression));
      decl->set_location_range((yylsp[-3]), (yylsp[-2]));

      (yyval.declarator_list) = new(ctx) ast_declarator_list((yyvsp[-4].fully_specified_type));
      (yyval.declarator_list)->set_location_range((yylsp[-4]), (yylsp[-2]));
      (yyval.declarator_list)->declarations.push_tail(&decl->link);
   }
#line 4282 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1117 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[-2].identifier), NULL, (yyvsp[0].expression));
      decl->set_location((yylsp[-2]));

      (yyval.declarator_list) = new(ctx) ast_declarator_list((yyvsp[-3].fully_specified_type));
      (yyval.declarator_list)->set_location_range((yylsp[-3]), (yylsp[-2]));
      (yyval.declarator_list)->declarations.push_tail(&decl->link);
   }
#line 4296 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1127 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[0].identifier), NULL, NULL);
      decl->set_location((yylsp[0]));

      (yyval.declarator_list) = new(ctx) ast_declarator_list(NULL);
      (yyval.declarator_list)->set_location_range((yylsp[-1]), (yylsp[0]));
      (yyval.declarator_list)->invariant = true;

      (yyval.declarator_list)->declarations.push_tail(&decl->link);
   }
#line 4312 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1139 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[0].identifier), NULL, NULL);
      decl->set_location((yylsp[0]));

      (yyval.declarator_list) = new(ctx) ast_declarator_list(NULL);
      (yyval.declarator_list)->set_location_range((yylsp[-1]), (yylsp[0]));
      (yyval.declarator_list)->precise = true;

      (yyval.declarator_list)->declarations.push_tail(&decl->link);
   }
#line 4328 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1154 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.fully_specified_type) = new(ctx) ast_fully_specified_type();
      (yyval.fully_specified_type)->set_location((yylsp[0]));
      (yyval.fully_specified_type)->specifier = (yyvsp[0].type_specifier);
   }
#line 4339 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1161 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.fully_specified_type) = new(ctx) ast_fully_specified_type();
      (yyval.fully_specified_type)->set_location_range((yylsp[-1]), (yylsp[0]));
      (yyval.fully_specified_type)->qualifier = (yyvsp[-1].type_qualifier);
      (yyval.fully_specified_type)->specifier = (yyvsp[0].type_specifier);
   }
#line 4351 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1172 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.type_qualifier) = (yyvsp[-1].type_qualifier);
   }
#line 4359 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1180 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.type_qualifier) = (yyvsp[-2].type_qualifier);
      if (!(yyval.type_qualifier).merge_qualifier(& (yylsp[0]), state, (yyvsp[0].type_qualifier))) {
         YYERROR;
      }
   }
#line 4370 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1189 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = (yyvsp[0].n); }
#line 4376 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1190 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.n) = (yyvsp[0].n); }
#line 4382 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1195 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));

      /* Layout qualifiers for ARB_fragment_coord_conventions. */
      if (!(yyval.type_qualifier).flags.i && (state->ARB_fragment_coord_conventions_enable ||
                          state->is_version(150, 0))) {
         if (match_layout_qualifier((yyvsp[0].identifier), "origin_upper_left", state) == 0) {
            (yyval.type_qualifier).flags.q.origin_upper_left = 1;
         } else if (match_layout_qualifier((yyvsp[0].identifier), "pixel_center_integer",
                                           state) == 0) {
            (yyval.type_qualifier).flags.q.pixel_center_integer = 1;
         }

         if ((yyval.type_qualifier).flags.i && state->ARB_fragment_coord_conventions_warn) {
            _mesa_glsl_warning(& (yylsp[0]), state,
                               "GL_ARB_fragment_coord_conventions layout "
                               "identifier `%s' used", (yyvsp[0].identifier));
         }
      }

      /* Layout qualifiers for AMD/ARB_conservative_depth. */
      if (!(yyval.type_qualifier).flags.i &&
          (state->AMD_conservative_depth_enable ||
           state->ARB_conservative_depth_enable)) {
         if (match_layout_qualifier((yyvsp[0].identifier), "depth_any", state) == 0) {
            (yyval.type_qualifier).flags.q.depth_any = 1;
         } else if (match_layout_qualifier((yyvsp[0].identifier), "depth_greater", state) == 0) {
            (yyval.type_qualifier).flags.q.depth_greater = 1;
         } else if (match_layout_qualifier((yyvsp[0].identifier), "depth_less", state) == 0) {
            (yyval.type_qualifier).flags.q.depth_less = 1;
         } else if (match_layout_qualifier((yyvsp[0].identifier), "depth_unchanged",
                                           state) == 0) {
            (yyval.type_qualifier).flags.q.depth_unchanged = 1;
         }

         if ((yyval.type_qualifier).flags.i && state->AMD_conservative_depth_warn) {
            _mesa_glsl_warning(& (yylsp[0]), state,
                               "GL_AMD_conservative_depth "
                               "layout qualifier `%s' is used", (yyvsp[0].identifier));
         }
         if ((yyval.type_qualifier).flags.i && state->ARB_conservative_depth_warn) {
            _mesa_glsl_warning(& (yylsp[0]), state,
                               "GL_ARB_conservative_depth "
                               "layout qualifier `%s' is used", (yyvsp[0].identifier));
         }
      }

      /* See also interface_block_layout_qualifier. */
      if (!(yyval.type_qualifier).flags.i && state->has_uniform_buffer_objects()) {
         if (match_layout_qualifier((yyvsp[0].identifier), "std140", state) == 0) {
            (yyval.type_qualifier).flags.q.std140 = 1;
         } else if (match_layout_qualifier((yyvsp[0].identifier), "shared", state) == 0) {
            (yyval.type_qualifier).flags.q.shared = 1;
         } else if (match_layout_qualifier((yyvsp[0].identifier), "column_major", state) == 0) {
            (yyval.type_qualifier).flags.q.column_major = 1;
         /* "row_major" is a reserved word in GLSL 1.30+. Its token is parsed
          * below in the interface_block_layout_qualifier rule.
          *
          * It is not a reserved word in GLSL ES 3.00, so it's handled here as
          * an identifier.
          *
          * Also, this takes care of alternate capitalizations of
          * "row_major" (which is necessary because layout qualifiers
          * are case-insensitive in desktop GLSL).
          */
         } else if (match_layout_qualifier((yyvsp[0].identifier), "row_major", state) == 0) {
            (yyval.type_qualifier).flags.q.row_major = 1;
         /* "packed" is a reserved word in GLSL, and its token is
          * parsed below in the interface_block_layout_qualifier rule.
          * However, we must take care of alternate capitalizations of
          * "packed", because layout qualifiers are case-insensitive
          * in desktop GLSL.
          */
         } else if (match_layout_qualifier((yyvsp[0].identifier), "packed", state) == 0) {
           (yyval.type_qualifier).flags.q.packed = 1;
         }

         if ((yyval.type_qualifier).flags.i && state->ARB_uniform_buffer_object_warn) {
            _mesa_glsl_warning(& (yylsp[0]), state,
                               "#version 140 / GL_ARB_uniform_buffer_object "
                               "layout qualifier `%s' is used", (yyvsp[0].identifier));
         }
      }

      /* Layout qualifiers for GLSL 1.50 geometry shaders. */
      if (!(yyval.type_qualifier).flags.i) {
         static const struct {
            const char *s;
            GLenum e;
         } map[] = {
                 { "points", GL_POINTS },
                 { "lines", GL_LINES },
                 { "lines_adjacency", GL_LINES_ADJACENCY },
                 { "line_strip", GL_LINE_STRIP },
                 { "triangles", GL_TRIANGLES },
                 { "triangles_adjacency", GL_TRIANGLES_ADJACENCY },
                 { "triangle_strip", GL_TRIANGLE_STRIP },
         };
         for (unsigned i = 0; i < ARRAY_SIZE(map); i++) {
            if (match_layout_qualifier((yyvsp[0].identifier), map[i].s, state) == 0) {
               (yyval.type_qualifier).flags.q.prim_type = 1;
               (yyval.type_qualifier).prim_type = map[i].e;
               break;
            }
         }

         if ((yyval.type_qualifier).flags.i && !state->is_version(150, 0)) {
            _mesa_glsl_error(& (yylsp[0]), state, "#version 150 layout "
                             "qualifier `%s' used", (yyvsp[0].identifier));
         }
      }

      /* Layout qualifiers for ARB_shader_image_load_store. */
      if (state->ARB_shader_image_load_store_enable ||
          state->is_version(420, 0)) {
         if (!(yyval.type_qualifier).flags.i) {
            static const struct {
               const char *name;
               GLenum format;
               glsl_base_type base_type;
            } map[] = {
               { "rgba32f", GL_RGBA32F, GLSL_TYPE_FLOAT },
               { "rgba16f", GL_RGBA16F, GLSL_TYPE_FLOAT },
               { "rg32f", GL_RG32F, GLSL_TYPE_FLOAT },
               { "rg16f", GL_RG16F, GLSL_TYPE_FLOAT },
               { "r11f_g11f_b10f", GL_R11F_G11F_B10F, GLSL_TYPE_FLOAT },
               { "r32f", GL_R32F, GLSL_TYPE_FLOAT },
               { "r16f", GL_R16F, GLSL_TYPE_FLOAT },
               { "rgba32ui", GL_RGBA32UI, GLSL_TYPE_UINT },
               { "rgba16ui", GL_RGBA16UI, GLSL_TYPE_UINT },
               { "rgb10_a2ui", GL_RGB10_A2UI, GLSL_TYPE_UINT },
               { "rgba8ui", GL_RGBA8UI, GLSL_TYPE_UINT },
               { "rg32ui", GL_RG32UI, GLSL_TYPE_UINT },
               { "rg16ui", GL_RG16UI, GLSL_TYPE_UINT },
               { "rg8ui", GL_RG8UI, GLSL_TYPE_UINT },
               { "r32ui", GL_R32UI, GLSL_TYPE_UINT },
               { "r16ui", GL_R16UI, GLSL_TYPE_UINT },
               { "r8ui", GL_R8UI, GLSL_TYPE_UINT },
               { "rgba32i", GL_RGBA32I, GLSL_TYPE_INT },
               { "rgba16i", GL_RGBA16I, GLSL_TYPE_INT },
               { "rgba8i", GL_RGBA8I, GLSL_TYPE_INT },
               { "rg32i", GL_RG32I, GLSL_TYPE_INT },
               { "rg16i", GL_RG16I, GLSL_TYPE_INT },
               { "rg8i", GL_RG8I, GLSL_TYPE_INT },
               { "r32i", GL_R32I, GLSL_TYPE_INT },
               { "r16i", GL_R16I, GLSL_TYPE_INT },
               { "r8i", GL_R8I, GLSL_TYPE_INT },
               { "rgba16", GL_RGBA16, GLSL_TYPE_FLOAT },
               { "rgb10_a2", GL_RGB10_A2, GLSL_TYPE_FLOAT },
               { "rgba8", GL_RGBA8, GLSL_TYPE_FLOAT },
               { "rg16", GL_RG16, GLSL_TYPE_FLOAT },
               { "rg8", GL_RG8, GLSL_TYPE_FLOAT },
               { "r16", GL_R16, GLSL_TYPE_FLOAT },
               { "r8", GL_R8, GLSL_TYPE_FLOAT },
               { "rgba16_snorm", GL_RGBA16_SNORM, GLSL_TYPE_FLOAT },
               { "rgba8_snorm", GL_RGBA8_SNORM, GLSL_TYPE_FLOAT },
               { "rg16_snorm", GL_RG16_SNORM, GLSL_TYPE_FLOAT },
               { "rg8_snorm", GL_RG8_SNORM, GLSL_TYPE_FLOAT },
               { "r16_snorm", GL_R16_SNORM, GLSL_TYPE_FLOAT },
               { "r8_snorm", GL_R8_SNORM, GLSL_TYPE_FLOAT }
            };

            for (unsigned i = 0; i < ARRAY_SIZE(map); i++) {
               if (match_layout_qualifier((yyvsp[0].identifier), map[i].name, state) == 0) {
                  (yyval.type_qualifier).flags.q.explicit_image_format = 1;
                  (yyval.type_qualifier).image_format = map[i].format;
                  (yyval.type_qualifier).image_base_type = map[i].base_type;
                  break;
               }
            }
         }

         if (!(yyval.type_qualifier).flags.i &&
             match_layout_qualifier((yyvsp[0].identifier), "early_fragment_tests", state) == 0) {
            /* From section 4.4.1.3 of the GLSL 4.50 specification
             * (Fragment Shader Inputs):
             *
             *  "Fragment shaders also allow the following layout
             *   qualifier on in only (not with variable declarations)
             *     layout-qualifier-id
             *        early_fragment_tests
             *   [...]"
             */
            if (state->stage != MESA_SHADER_FRAGMENT) {
               _mesa_glsl_error(& (yylsp[0]), state,
                                "early_fragment_tests layout qualifier only "
                                "valid in fragment shaders");
            }

            (yyval.type_qualifier).flags.q.early_fragment_tests = 1;
         }
      }

      if (!(yyval.type_qualifier).flags.i) {
         _mesa_glsl_error(& (yylsp[0]), state, "unrecognized layout identifier "
                          "`%s'", (yyvsp[0].identifier));
         YYERROR;
      }
   }
#line 4586 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1395 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));

      if (match_layout_qualifier("location", (yyvsp[-2].identifier), state) == 0) {
         (yyval.type_qualifier).flags.q.explicit_location = 1;

         if ((yyval.type_qualifier).flags.q.attribute == 1 &&
             state->ARB_explicit_attrib_location_warn) {
            _mesa_glsl_warning(& (yylsp[-2]), state,
                               "GL_ARB_explicit_attrib_location layout "
                               "identifier `%s' used", (yyvsp[-2].identifier));
         }

         if ((yyvsp[0].n) >= 0) {
            (yyval.type_qualifier).location = (yyvsp[0].n);
         } else {
             _mesa_glsl_error(& (yylsp[0]), state, "invalid location %d specified", (yyvsp[0].n));
             YYERROR;
         }
      }

      if (match_layout_qualifier("index", (yyvsp[-2].identifier), state) == 0) {
         (yyval.type_qualifier).flags.q.explicit_index = 1;

         if ((yyvsp[0].n) >= 0) {
            (yyval.type_qualifier).index = (yyvsp[0].n);
         } else {
            _mesa_glsl_error(& (yylsp[0]), state, "invalid index %d specified", (yyvsp[0].n));
            YYERROR;
         }
      }

      if ((state->ARB_shading_language_420pack_enable ||
           state->has_atomic_counters()) &&
          match_layout_qualifier("binding", (yyvsp[-2].identifier), state) == 0) {
         (yyval.type_qualifier).flags.q.explicit_binding = 1;
         (yyval.type_qualifier).binding = (yyvsp[0].n);
      }

      if (state->has_atomic_counters() &&
          match_layout_qualifier("offset", (yyvsp[-2].identifier), state) == 0) {
         (yyval.type_qualifier).flags.q.explicit_offset = 1;
         (yyval.type_qualifier).offset = (yyvsp[0].n);
      }

      if (match_layout_qualifier("max_vertices", (yyvsp[-2].identifier), state) == 0) {
         (yyval.type_qualifier).flags.q.max_vertices = 1;

         if ((yyvsp[0].n) < 0) {
            _mesa_glsl_error(& (yylsp[0]), state,
                             "invalid max_vertices %d specified", (yyvsp[0].n));
            YYERROR;
         } else {
            (yyval.type_qualifier).max_vertices = (yyvsp[0].n);
            if (!state->is_version(150, 0)) {
               _mesa_glsl_error(& (yylsp[0]), state,
                                "#version 150 max_vertices qualifier "
                                "specified", (yyvsp[0].n));
            }
         }
      }

      if (state->stage == MESA_SHADER_GEOMETRY) {
         if (match_layout_qualifier("stream", (yyvsp[-2].identifier), state) == 0 &&
             state->check_explicit_attrib_stream_allowed(& (yylsp[0]))) {
            (yyval.type_qualifier).flags.q.stream = 1;

            if ((yyvsp[0].n) < 0) {
               _mesa_glsl_error(& (yylsp[0]), state,
                                "invalid stream %d specified", (yyvsp[0].n));
               YYERROR;
            } else {
               (yyval.type_qualifier).flags.q.explicit_stream = 1;
               (yyval.type_qualifier).stream = (yyvsp[0].n);
            }
         }
      }

      static const char * const local_size_qualifiers[3] = {
         "local_size_x",
         "local_size_y",
         "local_size_z",
      };
      for (int i = 0; i < 3; i++) {
         if (match_layout_qualifier(local_size_qualifiers[i], (yyvsp[-2].identifier),
                                    state) == 0) {
            if ((yyvsp[0].n) <= 0) {
               _mesa_glsl_error(& (yylsp[0]), state,
                                "invalid %s of %d specified",
                                local_size_qualifiers[i], (yyvsp[0].n));
               YYERROR;
            } else if (!state->is_version(430, 0) &&
                       !state->ARB_compute_shader_enable) {
               _mesa_glsl_error(& (yylsp[0]), state,
                                "%s qualifier requires GLSL 4.30 or "
                                "ARB_compute_shader",
                                local_size_qualifiers[i]);
               YYERROR;
            } else {
               (yyval.type_qualifier).flags.q.local_size |= (1 << i);
               (yyval.type_qualifier).local_size[i] = (yyvsp[0].n);
            }
            break;
         }
      }

      if (match_layout_qualifier("invocations", (yyvsp[-2].identifier), state) == 0) {
         (yyval.type_qualifier).flags.q.invocations = 1;

         if ((yyvsp[0].n) <= 0) {
            _mesa_glsl_error(& (yylsp[0]), state,
                             "invalid invocations %d specified", (yyvsp[0].n));
            YYERROR;
         } else if ((yyvsp[0].n) > MAX_GEOMETRY_SHADER_INVOCATIONS) {
            _mesa_glsl_error(& (yylsp[0]), state,
                             "invocations (%d) exceeds "
                             "GL_MAX_GEOMETRY_SHADER_INVOCATIONS", (yyvsp[0].n));
            YYERROR;
         } else {
            (yyval.type_qualifier).invocations = (yyvsp[0].n);
            if (!state->is_version(400, 0) &&
                !state->ARB_gpu_shader5_enable) {
               _mesa_glsl_error(& (yylsp[0]), state,
                                "GL_ARB_gpu_shader5 invocations "
                                "qualifier specified", (yyvsp[0].n));
            }
         }
      }

      /* If the identifier didn't match any known layout identifiers,
       * emit an error.
       */
      if (!(yyval.type_qualifier).flags.i) {
         _mesa_glsl_error(& (yylsp[-2]), state, "unrecognized layout identifier "
                          "`%s'", (yyvsp[-2].identifier));
         YYERROR;
      }
   }
#line 4729 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1534 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.type_qualifier) = (yyvsp[0].type_qualifier);
      /* Layout qualifiers for ARB_uniform_buffer_object. */
      if ((yyval.type_qualifier).flags.q.uniform && !state->has_uniform_buffer_objects()) {
         _mesa_glsl_error(& (yylsp[0]), state,
                          "#version 140 / GL_ARB_uniform_buffer_object "
                          "layout qualifier `%s' is used", (yyvsp[0].type_qualifier));
      } else if ((yyval.type_qualifier).flags.q.uniform && state->ARB_uniform_buffer_object_warn) {
         _mesa_glsl_warning(& (yylsp[0]), state,
                            "#version 140 / GL_ARB_uniform_buffer_object "
                            "layout qualifier `%s' is used", (yyvsp[0].type_qualifier));
      }
   }
#line 4747 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1560 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.row_major = 1;
   }
#line 4756 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1565 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.packed = 1;
   }
#line 4765 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1573 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.smooth = 1;
   }
#line 4774 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1578 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.flat = 1;
   }
#line 4783 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1583 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.noperspective = 1;
   }
#line 4792 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1592 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.invariant = 1;
   }
#line 4801 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1597 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.precise = 1;
   }
#line 4810 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1607 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(&(yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).precision = (yyvsp[0].n);
   }
#line 4819 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1625 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[0].type_qualifier).flags.q.precise)
         _mesa_glsl_error(&(yylsp[-1]), state, "duplicate \"precise\" qualifier");

      (yyval.type_qualifier) = (yyvsp[0].type_qualifier);
      (yyval.type_qualifier).flags.q.precise = 1;
   }
#line 4831 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1633 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[0].type_qualifier).flags.q.invariant)
         _mesa_glsl_error(&(yylsp[-1]), state, "duplicate \"invariant\" qualifier");

      if (!state->ARB_shading_language_420pack_enable && (yyvsp[0].type_qualifier).flags.q.precise)
         _mesa_glsl_error(&(yylsp[-1]), state,
                          "\"invariant\" must come after \"precise\"");

      (yyval.type_qualifier) = (yyvsp[0].type_qualifier);
      (yyval.type_qualifier).flags.q.invariant = 1;

      /* GLSL ES 3.00 spec, section 4.6.1 "The Invariant Qualifier":
       *
       * "Only variables output from a shader can be candidates for invariance.
       * This includes user-defined output variables and the built-in output
       * variables. As only outputs can be declared as invariant, an invariant
       * output from one shader stage will still match an input of a subsequent
       * stage without the input being declared as invariant."
       */
      if (state->es_shader && state->language_version >= 300 && (yyval.type_qualifier).flags.q.in)
         _mesa_glsl_error(&(yylsp[-1]), state, "invariant qualifiers cannot be used with shader inputs");
   }
#line 4858 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1656 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      /* Section 4.3 of the GLSL 1.40 specification states:
       * "...qualified with one of these interpolation qualifiers"
       *
       * GLSL 1.30 claims to allow "one or more", but insists that:
       * "These interpolation qualifiers may only precede the qualifiers in,
       *  centroid in, out, or centroid out in a declaration."
       *
       * ...which means that e.g. smooth can't precede smooth, so there can be
       * only one after all, and the 1.40 text is a clarification, not a change.
       */
      if ((yyvsp[0].type_qualifier).has_interpolation())
         _mesa_glsl_error(&(yylsp[-1]), state, "duplicate interpolation qualifier");

      if (!state->ARB_shading_language_420pack_enable &&
          ((yyvsp[0].type_qualifier).flags.q.precise || (yyvsp[0].type_qualifier).flags.q.invariant)) {
         _mesa_glsl_error(&(yylsp[-1]), state, "interpolation qualifiers must come "
                          "after \"precise\" or \"invariant\"");
      }

      (yyval.type_qualifier) = (yyvsp[-1].type_qualifier);
      (yyval.type_qualifier).merge_qualifier(&(yylsp[-1]), state, (yyvsp[0].type_qualifier));
   }
#line 4886 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1680 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      /* In the absence of ARB_shading_language_420pack, layout qualifiers may
       * appear no later than auxiliary storage qualifiers. There is no
       * particularly clear spec language mandating this, but in all examples
       * the layout qualifier precedes the storage qualifier.
       *
       * We allow combinations of layout with interpolation, invariant or
       * precise qualifiers since these are useful in ARB_separate_shader_objects.
       * There is no clear spec guidance on this either.
       */
      if (!state->ARB_shading_language_420pack_enable && (yyvsp[0].type_qualifier).has_layout())
         _mesa_glsl_error(&(yylsp[-1]), state, "duplicate layout(...) qualifiers");

      (yyval.type_qualifier) = (yyvsp[-1].type_qualifier);
      (yyval.type_qualifier).merge_qualifier(&(yylsp[-1]), state, (yyvsp[0].type_qualifier));
   }
#line 4907 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1697 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[0].type_qualifier).has_auxiliary_storage()) {
         _mesa_glsl_error(&(yylsp[-1]), state,
                          "duplicate auxiliary storage qualifier (centroid or sample)");
      }

      if (!state->ARB_shading_language_420pack_enable &&
          ((yyvsp[0].type_qualifier).flags.q.precise || (yyvsp[0].type_qualifier).flags.q.invariant ||
           (yyvsp[0].type_qualifier).has_interpolation() || (yyvsp[0].type_qualifier).has_layout())) {
         _mesa_glsl_error(&(yylsp[-1]), state, "auxiliary storage qualifiers must come "
                          "just before storage qualifiers");
      }
      (yyval.type_qualifier) = (yyvsp[-1].type_qualifier);
      (yyval.type_qualifier).merge_qualifier(&(yylsp[-1]), state, (yyvsp[0].type_qualifier));
   }
#line 4927 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1713 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      /* Section 4.3 of the GLSL 1.20 specification states:
       * "Variable declarations may have a storage qualifier specified..."
       *  1.30 clarifies this to "may have one storage qualifier".
       */
      if ((yyvsp[0].type_qualifier).has_storage())
         _mesa_glsl_error(&(yylsp[-1]), state, "duplicate storage qualifier");

      if (!state->ARB_shading_language_420pack_enable &&
          ((yyvsp[0].type_qualifier).flags.q.precise || (yyvsp[0].type_qualifier).flags.q.invariant || (yyvsp[0].type_qualifier).has_interpolation() ||
           (yyvsp[0].type_qualifier).has_layout() || (yyvsp[0].type_qualifier).has_auxiliary_storage())) {
         _mesa_glsl_error(&(yylsp[-1]), state, "storage qualifiers must come after "
                          "precise, invariant, interpolation, layout and auxiliary "
                          "storage qualifiers");
      }

      (yyval.type_qualifier) = (yyvsp[-1].type_qualifier);
      (yyval.type_qualifier).merge_qualifier(&(yylsp[-1]), state, (yyvsp[0].type_qualifier));
   }
#line 4951 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1733 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[0].type_qualifier).precision != ast_precision_none)
         _mesa_glsl_error(&(yylsp[-1]), state, "duplicate precision qualifier");

      if (!state->ARB_shading_language_420pack_enable && (yyvsp[0].type_qualifier).flags.i != 0)
         _mesa_glsl_error(&(yylsp[-1]), state, "precision qualifiers must come last");

      (yyval.type_qualifier) = (yyvsp[0].type_qualifier);
      (yyval.type_qualifier).precision = (yyvsp[-1].n);
   }
#line 4966 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1744 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.type_qualifier) = (yyvsp[-1].type_qualifier);
      (yyval.type_qualifier).merge_qualifier(&(yylsp[-1]), state, (yyvsp[0].type_qualifier));
   }
#line 4975 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1752 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.centroid = 1;
   }
#line 4984 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1757 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.sample = 1;
   }
#line 4993 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1765 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.constant = 1;
   }
#line 5002 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1770 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.attribute = 1;
   }
#line 5011 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1775 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.varying = 1;
   }
#line 5020 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1780 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.in = 1;
   }
#line 5029 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1785 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.out = 1;

      if (state->stage == MESA_SHADER_GEOMETRY &&
          state->has_explicit_attrib_stream()) {
         /* Section 4.3.8.2 (Output Layout Qualifiers) of the GLSL 4.00
          * spec says:
          *
          *     "If the block or variable is declared with the stream
          *     identifier, it is associated with the specified stream;
          *     otherwise, it is associated with the current default stream."
          */
          (yyval.type_qualifier).flags.q.stream = 1;
          (yyval.type_qualifier).flags.q.explicit_stream = 0;
          (yyval.type_qualifier).stream = state->out_qualifier->stream;
      }
   }
#line 5052 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1804 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.uniform = 1;
   }
#line 5061 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1812 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.coherent = 1;
   }
#line 5070 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1817 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q._volatile = 1;
   }
#line 5079 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1822 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      STATIC_ASSERT(sizeof((yyval.type_qualifier).flags.q) <= sizeof((yyval.type_qualifier).flags.i));
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.restrict_flag = 1;
   }
#line 5089 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1828 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.read_only = 1;
   }
#line 5098 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1833 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.write_only = 1;
   }
#line 5107 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1841 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.array_specifier) = new(ctx) ast_array_specifier((yylsp[-1]));
      (yyval.array_specifier)->set_location_range((yylsp[-1]), (yylsp[0]));
   }
#line 5117 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1847 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.array_specifier) = new(ctx) ast_array_specifier((yylsp[-2]), (yyvsp[-1].expression));
      (yyval.array_specifier)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 5127 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1853 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.array_specifier) = (yyvsp[-2].array_specifier);

      if (!state->ARB_arrays_of_arrays_enable) {
         _mesa_glsl_error(& (yylsp[-2]), state,
                          "GL_ARB_arrays_of_arrays "
                          "required for defining arrays of arrays");
      } else {
         _mesa_glsl_error(& (yylsp[-2]), state,
                          "only the outermost array dimension can "
                          "be unsized");
      }
   }
#line 5145 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1867 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.array_specifier) = (yyvsp[-3].array_specifier);

      if (!state->ARB_arrays_of_arrays_enable) {
         _mesa_glsl_error(& (yylsp[-3]), state,
                          "GL_ARB_arrays_of_arrays "
                          "required for defining arrays of arrays");
      }

      (yyval.array_specifier)->add_dimension((yyvsp[-1].expression));
   }
#line 5161 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1883 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.type_specifier) = (yyvsp[-1].type_specifier);
      (yyval.type_specifier)->array_specifier = (yyvsp[0].array_specifier);
   }
#line 5170 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1891 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.type_specifier) = new(ctx) ast_type_specifier((yyvsp[0].identifier));
      (yyval.type_specifier)->set_location((yylsp[0]));
   }
#line 5180 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1897 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.type_specifier) = new(ctx) ast_type_specifier((yyvsp[0].struct_specifier));
      (yyval.type_specifier)->set_location((yylsp[0]));
   }
#line 5190 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1903 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.type_specifier) = new(ctx) ast_type_specifier((yyvsp[0].identifier));
      (yyval.type_specifier)->set_location((yylsp[0]));
   }
#line 5200 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1911 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "void"; }
#line 5206 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1912 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "float"; }
#line 5212 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1913 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "double"; }
#line 5218 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1914 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "int"; }
#line 5224 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1915 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uint"; }
#line 5230 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1916 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "bool"; }
#line 5236 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 1917 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "vec2"; }
#line 5242 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1918 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "vec3"; }
#line 5248 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1919 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "vec4"; }
#line 5254 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1920 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "bvec2"; }
#line 5260 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1921 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "bvec3"; }
#line 5266 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1922 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "bvec4"; }
#line 5272 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1923 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "ivec2"; }
#line 5278 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1924 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "ivec3"; }
#line 5284 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1925 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "ivec4"; }
#line 5290 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1926 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uvec2"; }
#line 5296 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1927 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uvec3"; }
#line 5302 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1928 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uvec4"; }
#line 5308 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1929 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dvec2"; }
#line 5314 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1930 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dvec3"; }
#line 5320 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1931 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dvec4"; }
#line 5326 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1932 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "mat2"; }
#line 5332 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1933 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "mat2x3"; }
#line 5338 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1934 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "mat2x4"; }
#line 5344 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1935 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "mat3x2"; }
#line 5350 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1936 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "mat3"; }
#line 5356 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1937 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "mat3x4"; }
#line 5362 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1938 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "mat4x2"; }
#line 5368 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1939 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "mat4x3"; }
#line 5374 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1940 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "mat4"; }
#line 5380 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1941 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dmat2"; }
#line 5386 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1942 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dmat2x3"; }
#line 5392 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1943 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dmat2x4"; }
#line 5398 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1944 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dmat3x2"; }
#line 5404 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1945 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dmat3"; }
#line 5410 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1946 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dmat3x4"; }
#line 5416 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1947 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dmat4x2"; }
#line 5422 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1948 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dmat4x3"; }
#line 5428 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1949 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "dmat4"; }
#line 5434 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1950 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler1D"; }
#line 5440 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1951 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler2D"; }
#line 5446 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1952 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler2DRect"; }
#line 5452 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1953 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler3D"; }
#line 5458 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1954 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "samplerCube"; }
#line 5464 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1955 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "samplerExternalOES"; }
#line 5470 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1956 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler1DShadow"; }
#line 5476 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1957 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler2DShadow"; }
#line 5482 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1958 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler2DRectShadow"; }
#line 5488 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1959 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "samplerCubeShadow"; }
#line 5494 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1960 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler1DArray"; }
#line 5500 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1961 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler2DArray"; }
#line 5506 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1962 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler1DArrayShadow"; }
#line 5512 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1963 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler2DArrayShadow"; }
#line 5518 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1964 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "samplerBuffer"; }
#line 5524 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1965 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "samplerCubeArray"; }
#line 5530 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1966 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "samplerCubeArrayShadow"; }
#line 5536 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1967 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isampler1D"; }
#line 5542 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1968 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isampler2D"; }
#line 5548 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1969 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isampler2DRect"; }
#line 5554 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1970 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isampler3D"; }
#line 5560 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1971 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isamplerCube"; }
#line 5566 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1972 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isampler1DArray"; }
#line 5572 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1973 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isampler2DArray"; }
#line 5578 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1974 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isamplerBuffer"; }
#line 5584 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1975 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isamplerCubeArray"; }
#line 5590 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1976 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usampler1D"; }
#line 5596 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1977 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usampler2D"; }
#line 5602 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1978 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usampler2DRect"; }
#line 5608 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1979 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usampler3D"; }
#line 5614 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1980 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usamplerCube"; }
#line 5620 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1981 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usampler1DArray"; }
#line 5626 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1982 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usampler2DArray"; }
#line 5632 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1983 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usamplerBuffer"; }
#line 5638 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1984 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usamplerCubeArray"; }
#line 5644 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1985 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler2DMS"; }
#line 5650 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1986 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isampler2DMS"; }
#line 5656 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1987 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usampler2DMS"; }
#line 5662 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1988 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "sampler2DMSArray"; }
#line 5668 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1989 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "isampler2DMSArray"; }
#line 5674 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1990 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "usampler2DMSArray"; }
#line 5680 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1991 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "image1D"; }
#line 5686 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1992 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "image2D"; }
#line 5692 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1993 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "image3D"; }
#line 5698 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1994 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "image2DRect"; }
#line 5704 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1995 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "imageCube"; }
#line 5710 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1996 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "imageBuffer"; }
#line 5716 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1997 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "image1DArray"; }
#line 5722 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1998 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "image2DArray"; }
#line 5728 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1999 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "imageCubeArray"; }
#line 5734 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 2000 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "image2DMS"; }
#line 5740 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 2001 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "image2DMSArray"; }
#line 5746 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 2002 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimage1D"; }
#line 5752 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 2003 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimage2D"; }
#line 5758 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 2004 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimage3D"; }
#line 5764 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 2005 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimage2DRect"; }
#line 5770 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 2006 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimageCube"; }
#line 5776 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 2007 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimageBuffer"; }
#line 5782 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 2008 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimage1DArray"; }
#line 5788 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 2009 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimage2DArray"; }
#line 5794 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 2010 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimageCubeArray"; }
#line 5800 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 2011 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimage2DMS"; }
#line 5806 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 2012 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "iimage2DMSArray"; }
#line 5812 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 2013 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimage1D"; }
#line 5818 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 2014 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimage2D"; }
#line 5824 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 2015 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimage3D"; }
#line 5830 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 2016 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimage2DRect"; }
#line 5836 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 2017 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimageCube"; }
#line 5842 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 2018 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimageBuffer"; }
#line 5848 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 2019 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimage1DArray"; }
#line 5854 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 2020 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimage2DArray"; }
#line 5860 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 2021 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimageCubeArray"; }
#line 5866 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 2022 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimage2DMS"; }
#line 5872 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 2023 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "uimage2DMSArray"; }
#line 5878 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 2024 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.identifier) = "atomic_uint"; }
#line 5884 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 2029 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      state->check_precision_qualifiers_allowed(&(yylsp[0]));
      (yyval.n) = ast_precision_high;
   }
#line 5893 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 2034 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      state->check_precision_qualifiers_allowed(&(yylsp[0]));
      (yyval.n) = ast_precision_medium;
   }
#line 5902 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 2039 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      state->check_precision_qualifiers_allowed(&(yylsp[0]));
      (yyval.n) = ast_precision_low;
   }
#line 5911 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 2047 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.struct_specifier) = new(ctx) ast_struct_specifier((yyvsp[-3].identifier), (yyvsp[-1].declarator_list));
      (yyval.struct_specifier)->set_location_range((yylsp[-3]), (yylsp[0]));
      state->symbols->add_type((yyvsp[-3].identifier), glsl_type::void_type);
   }
#line 5922 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 2054 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.struct_specifier) = new(ctx) ast_struct_specifier(NULL, (yyvsp[-1].declarator_list));
      (yyval.struct_specifier)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 5932 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 2063 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.declarator_list) = (yyvsp[0].declarator_list);
      (yyvsp[0].declarator_list)->link.self_link();
   }
#line 5941 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 2068 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.declarator_list) = (yyvsp[-1].declarator_list);
      (yyval.declarator_list)->link.insert_before(& (yyvsp[0].declarator_list)->link);
   }
#line 5950 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 2076 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_fully_specified_type *const type = (yyvsp[-2].fully_specified_type);
      type->set_location((yylsp[-2]));

      if (type->qualifier.flags.i != 0)
         _mesa_glsl_error(&(yylsp[-2]), state,
			  "only precision qualifiers may be applied to "
			  "structure members");

      (yyval.declarator_list) = new(ctx) ast_declarator_list(type);
      (yyval.declarator_list)->set_location((yylsp[-1]));

      (yyval.declarator_list)->declarations.push_degenerate_list_at_head(& (yyvsp[-1].declaration)->link);
   }
#line 5970 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 2095 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.declaration) = (yyvsp[0].declaration);
      (yyvsp[0].declaration)->link.self_link();
   }
#line 5979 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 2100 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.declaration) = (yyvsp[-2].declaration);
      (yyval.declaration)->link.insert_before(& (yyvsp[0].declaration)->link);
   }
#line 5988 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 2108 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.declaration) = new(ctx) ast_declaration((yyvsp[0].identifier), NULL, NULL);
      (yyval.declaration)->set_location((yylsp[0]));
   }
#line 5998 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 2114 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.declaration) = new(ctx) ast_declaration((yyvsp[-1].identifier), (yyvsp[0].array_specifier), NULL);
      (yyval.declaration)->set_location_range((yylsp[-1]), (yylsp[0]));
   }
#line 6008 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 2124 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.expression) = (yyvsp[-1].expression);
   }
#line 6016 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 2128 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.expression) = (yyvsp[-2].expression);
   }
#line 6024 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 2135 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.expression) = new(ctx) ast_aggregate_initializer();
      (yyval.expression)->set_location((yylsp[0]));
      (yyval.expression)->expressions.push_tail(& (yyvsp[0].expression)->link);
   }
#line 6035 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 2142 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyvsp[-2].expression)->expressions.push_tail(& (yyvsp[0].expression)->link);
   }
#line 6043 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 2154 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.node) = (ast_node *) (yyvsp[0].compound_statement); }
#line 6049 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 2169 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.compound_statement) = new(ctx) ast_compound_statement(true, NULL);
      (yyval.compound_statement)->set_location_range((yylsp[-1]), (yylsp[0]));
   }
#line 6059 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 2175 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      state->symbols->push_scope();
   }
#line 6067 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 2179 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.compound_statement) = new(ctx) ast_compound_statement(true, (yyvsp[-1].node));
      (yyval.compound_statement)->set_location_range((yylsp[-3]), (yylsp[0]));
      state->symbols->pop_scope();
   }
#line 6078 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 2188 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.node) = (ast_node *) (yyvsp[0].compound_statement); }
#line 6084 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 2194 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.compound_statement) = new(ctx) ast_compound_statement(false, NULL);
      (yyval.compound_statement)->set_location_range((yylsp[-1]), (yylsp[0]));
   }
#line 6094 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 2200 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.compound_statement) = new(ctx) ast_compound_statement(false, (yyvsp[-1].node));
      (yyval.compound_statement)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 6104 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 2209 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[0].node) == NULL) {
         _mesa_glsl_error(& (yylsp[0]), state, "<nil> statement");
         assert((yyvsp[0].node) != NULL);
      }

      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->link.self_link();
   }
#line 6118 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 2219 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if ((yyvsp[0].node) == NULL) {
         _mesa_glsl_error(& (yylsp[0]), state, "<nil> statement");
         assert((yyvsp[0].node) != NULL);
      }
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->link.insert_before(& (yyvsp[0].node)->link);
   }
#line 6131 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 2231 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.node) = new(ctx) ast_expression_statement(NULL);
      (yyval.node)->set_location((yylsp[0]));
   }
#line 6141 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 2237 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.node) = new(ctx) ast_expression_statement((yyvsp[-1].expression));
      (yyval.node)->set_location((yylsp[-1]));
   }
#line 6151 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 2246 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.node) = new(state) ast_selection_statement((yyvsp[-2].expression), (yyvsp[0].selection_rest_statement).then_statement,
                                              (yyvsp[0].selection_rest_statement).else_statement);
      (yyval.node)->set_location_range((yylsp[-4]), (yylsp[0]));
   }
#line 6161 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 2255 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.selection_rest_statement).then_statement = (yyvsp[-2].node);
      (yyval.selection_rest_statement).else_statement = (yyvsp[0].node);
   }
#line 6170 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 2260 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.selection_rest_statement).then_statement = (yyvsp[0].node);
      (yyval.selection_rest_statement).else_statement = NULL;
   }
#line 6179 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 2268 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.node) = (ast_node *) (yyvsp[0].expression);
   }
#line 6187 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 2272 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_declaration *decl = new(ctx) ast_declaration((yyvsp[-2].identifier), NULL, (yyvsp[0].expression));
      ast_declarator_list *declarator = new(ctx) ast_declarator_list((yyvsp[-3].fully_specified_type));
      decl->set_location_range((yylsp[-2]), (yylsp[0]));
      declarator->set_location((yylsp[-3]));

      declarator->declarations.push_tail(&decl->link);
      (yyval.node) = declarator;
   }
#line 6202 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 2290 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.node) = new(state) ast_switch_statement((yyvsp[-2].expression), (yyvsp[0].switch_body));
      (yyval.node)->set_location_range((yylsp[-4]), (yylsp[0]));
   }
#line 6211 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 2298 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.switch_body) = new(state) ast_switch_body(NULL);
      (yyval.switch_body)->set_location_range((yylsp[-1]), (yylsp[0]));
   }
#line 6220 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 2303 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.switch_body) = new(state) ast_switch_body((yyvsp[-1].case_statement_list));
      (yyval.switch_body)->set_location_range((yylsp[-2]), (yylsp[0]));
   }
#line 6229 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 2311 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.case_label) = new(state) ast_case_label((yyvsp[-1].expression));
      (yyval.case_label)->set_location((yylsp[-1]));
   }
#line 6238 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 2316 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.case_label) = new(state) ast_case_label(NULL);
      (yyval.case_label)->set_location((yylsp[0]));
   }
#line 6247 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 2324 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      ast_case_label_list *labels = new(state) ast_case_label_list();

      labels->labels.push_tail(& (yyvsp[0].case_label)->link);
      (yyval.case_label_list) = labels;
      (yyval.case_label_list)->set_location((yylsp[0]));
   }
#line 6259 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 2332 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.case_label_list) = (yyvsp[-1].case_label_list);
      (yyval.case_label_list)->labels.push_tail(& (yyvsp[0].case_label)->link);
   }
#line 6268 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 2340 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      ast_case_statement *stmts = new(state) ast_case_statement((yyvsp[-1].case_label_list));
      stmts->set_location((yylsp[0]));

      stmts->stmts.push_tail(& (yyvsp[0].node)->link);
      (yyval.case_statement) = stmts;
   }
#line 6280 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 2348 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.case_statement) = (yyvsp[-1].case_statement);
      (yyval.case_statement)->stmts.push_tail(& (yyvsp[0].node)->link);
   }
#line 6289 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 2356 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      ast_case_statement_list *cases= new(state) ast_case_statement_list();
      cases->set_location((yylsp[0]));

      cases->cases.push_tail(& (yyvsp[0].case_statement)->link);
      (yyval.case_statement_list) = cases;
   }
#line 6301 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 2364 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.case_statement_list) = (yyvsp[-1].case_statement_list);
      (yyval.case_statement_list)->cases.push_tail(& (yyvsp[0].case_statement)->link);
   }
#line 6310 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 2372 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.node) = new(ctx) ast_iteration_statement(ast_iteration_statement::ast_while,
                                            NULL, (yyvsp[-2].node), NULL, (yyvsp[0].node));
      (yyval.node)->set_location_range((yylsp[-4]), (yylsp[-1]));
   }
#line 6321 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 2379 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.node) = new(ctx) ast_iteration_statement(ast_iteration_statement::ast_do_while,
                                            NULL, (yyvsp[-2].expression), NULL, (yyvsp[-5].node));
      (yyval.node)->set_location_range((yylsp[-6]), (yylsp[-1]));
   }
#line 6332 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 2386 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.node) = new(ctx) ast_iteration_statement(ast_iteration_statement::ast_for,
                                            (yyvsp[-3].node), (yyvsp[-2].for_rest_statement).cond, (yyvsp[-2].for_rest_statement).rest, (yyvsp[0].node));
      (yyval.node)->set_location_range((yylsp[-5]), (yylsp[0]));
   }
#line 6343 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 2402 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.node) = NULL;
   }
#line 6351 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 2409 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.for_rest_statement).cond = (yyvsp[-1].node);
      (yyval.for_rest_statement).rest = NULL;
   }
#line 6360 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 2414 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.for_rest_statement).cond = (yyvsp[-2].node);
      (yyval.for_rest_statement).rest = (yyvsp[0].expression);
   }
#line 6369 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 2423 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.node) = new(ctx) ast_jump_statement(ast_jump_statement::ast_continue, NULL);
      (yyval.node)->set_location((yylsp[-1]));
   }
#line 6379 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 2429 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.node) = new(ctx) ast_jump_statement(ast_jump_statement::ast_break, NULL);
      (yyval.node)->set_location((yylsp[-1]));
   }
#line 6389 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 2435 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.node) = new(ctx) ast_jump_statement(ast_jump_statement::ast_return, NULL);
      (yyval.node)->set_location((yylsp[-1]));
   }
#line 6399 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 2441 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.node) = new(ctx) ast_jump_statement(ast_jump_statement::ast_return, (yyvsp[-1].expression));
      (yyval.node)->set_location_range((yylsp[-2]), (yylsp[-1]));
   }
#line 6409 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 2447 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.node) = new(ctx) ast_jump_statement(ast_jump_statement::ast_discard, NULL);
      (yyval.node)->set_location((yylsp[-1]));
   }
#line 6419 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 2455 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].function_definition); }
#line 6425 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 2456 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 6431 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 2457 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.node) = NULL; }
#line 6437 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 2458 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 6443 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 2463 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      (yyval.function_definition) = new(ctx) ast_function_definition();
      (yyval.function_definition)->set_location_range((yylsp[-1]), (yylsp[0]));
      (yyval.function_definition)->prototype = (yyvsp[-1].function);
      (yyval.function_definition)->body = (yyvsp[0].compound_statement);

      state->symbols->pop_scope();
   }
#line 6457 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 2477 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.node) = (yyvsp[0].interface_block);
   }
#line 6465 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 2481 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      ast_interface_block *block = (yyvsp[0].interface_block);
      if (!block->layout.merge_qualifier(& (yylsp[-1]), state, (yyvsp[-1].type_qualifier))) {
         YYERROR;
      }

      foreach_list_typed (ast_declarator_list, member, link, &block->declarations) {
         ast_type_qualifier& qualifier = member->type->qualifier;
         if (qualifier.flags.q.stream && qualifier.stream != block->layout.stream) {
               _mesa_glsl_error(& (yylsp[-1]), state,
                             "stream layout qualifier on "
                             "interface block member does not match "
                             "the interface block (%d vs %d)",
                             qualifier.stream, block->layout.stream);
               YYERROR;
         }
      }
      (yyval.node) = block;
   }
#line 6489 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 2504 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      ast_interface_block *const block = (yyvsp[-1].interface_block);

      block->block_name = (yyvsp[-5].identifier);
      block->declarations.push_degenerate_list_at_head(& (yyvsp[-3].declarator_list)->link);

      if ((yyvsp[-6].type_qualifier).flags.q.uniform) {
         if (!state->has_uniform_buffer_objects()) {
            _mesa_glsl_error(& (yylsp[-6]), state,
                             "#version 140 / GL_ARB_uniform_buffer_object "
                             "required for defining uniform blocks");
         } else if (state->ARB_uniform_buffer_object_warn) {
            _mesa_glsl_warning(& (yylsp[-6]), state,
                               "#version 140 / GL_ARB_uniform_buffer_object "
                               "required for defining uniform blocks");
         }
      } else {
         if (state->es_shader || state->language_version < 150) {
            _mesa_glsl_error(& (yylsp[-6]), state,
                             "#version 150 required for using "
                             "interface blocks");
         }
      }

      /* From the GLSL 1.50.11 spec, section 4.3.7 ("Interface Blocks"):
       * "It is illegal to have an input block in a vertex shader
       *  or an output block in a fragment shader"
       */
      if ((state->stage == MESA_SHADER_VERTEX) && (yyvsp[-6].type_qualifier).flags.q.in) {
         _mesa_glsl_error(& (yylsp[-6]), state,
                          "`in' interface block is not allowed for "
                          "a vertex shader");
      } else if ((state->stage == MESA_SHADER_FRAGMENT) && (yyvsp[-6].type_qualifier).flags.q.out) {
         _mesa_glsl_error(& (yylsp[-6]), state,
                          "`out' interface block is not allowed for "
                          "a fragment shader");
      }

      /* Since block arrays require names, and both features are added in
       * the same language versions, we don't have to explicitly
       * version-check both things.
       */
      if (block->instance_name != NULL) {
         state->check_version(150, 300, & (yylsp[-6]), "interface blocks with "
                               "an instance name are not allowed");
      }

      uint64_t interface_type_mask;
      struct ast_type_qualifier temp_type_qualifier;

      /* Get a bitmask containing only the in/out/uniform flags, allowing us
       * to ignore other irrelevant flags like interpolation qualifiers.
       */
      temp_type_qualifier.flags.i = 0;
      temp_type_qualifier.flags.q.uniform = true;
      temp_type_qualifier.flags.q.in = true;
      temp_type_qualifier.flags.q.out = true;
      interface_type_mask = temp_type_qualifier.flags.i;

      /* Get the block's interface qualifier.  The interface_qualifier
       * production rule guarantees that only one bit will be set (and
       * it will be in/out/uniform).
       */
      uint64_t block_interface_qualifier = (yyvsp[-6].type_qualifier).flags.i;

      block->layout.flags.i |= block_interface_qualifier;

      if (state->stage == MESA_SHADER_GEOMETRY &&
          state->has_explicit_attrib_stream()) {
         /* Assign global layout's stream value. */
         block->layout.flags.q.stream = 1;
         block->layout.flags.q.explicit_stream = 0;
         block->layout.stream = state->out_qualifier->stream;
      }

      foreach_list_typed (ast_declarator_list, member, link, &block->declarations) {
         ast_type_qualifier& qualifier = member->type->qualifier;
         if ((qualifier.flags.i & interface_type_mask) == 0) {
            /* GLSLangSpec.1.50.11, 4.3.7 (Interface Blocks):
             * "If no optional qualifier is used in a member declaration, the
             *  qualifier of the variable is just in, out, or uniform as declared
             *  by interface-qualifier."
             */
            qualifier.flags.i |= block_interface_qualifier;
         } else if ((qualifier.flags.i & interface_type_mask) !=
                    block_interface_qualifier) {
            /* GLSLangSpec.1.50.11, 4.3.7 (Interface Blocks):
             * "If optional qualifiers are used, they can include interpolation
             *  and storage qualifiers and they must declare an input, output,
             *  or uniform variable consistent with the interface qualifier of
             *  the block."
             */
            _mesa_glsl_error(& (yylsp[-6]), state,
                             "uniform/in/out qualifier on "
                             "interface block member does not match "
                             "the interface block");
         }

         /* From GLSL ES 3.0, chapter 4.3.7 "Interface Blocks":
          *
          * "GLSL ES 3.0 does not support interface blocks for shader inputs or
          * outputs."
          *
          * And from GLSL ES 3.0, chapter 4.6.1 "The invariant qualifier":.
          *
          * "Only variables output from a shader can be candidates for
          * invariance."
          *
          * From GLSL 4.40 and GLSL 1.50, section "Interface Blocks":
          *
          * "If optional qualifiers are used, they can include interpolation
          * qualifiers, auxiliary storage qualifiers, and storage qualifiers
          * and they must declare an input, output, or uniform member
          * consistent with the interface qualifier of the block"
          */
         if (qualifier.flags.q.invariant)
            _mesa_glsl_error(&(yylsp[-6]), state,
                             "invariant qualifiers cannot be used "
                             "with interface blocks members");
      }

      (yyval.interface_block) = block;
   }
#line 6617 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 2631 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.in = 1;
   }
#line 6626 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 2636 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.out = 1;
   }
#line 6635 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 2641 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      memset(& (yyval.type_qualifier), 0, sizeof((yyval.type_qualifier)));
      (yyval.type_qualifier).flags.q.uniform = 1;
   }
#line 6644 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 2649 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.interface_block) = new(state) ast_interface_block(*state->default_uniform_qualifier,
                                          NULL, NULL);
   }
#line 6653 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 2654 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.interface_block) = new(state) ast_interface_block(*state->default_uniform_qualifier,
                                          (yyvsp[0].identifier), NULL);
      (yyval.interface_block)->set_location((yylsp[0]));
   }
#line 6663 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 2660 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.interface_block) = new(state) ast_interface_block(*state->default_uniform_qualifier,
                                          (yyvsp[-1].identifier), (yyvsp[0].array_specifier));
      (yyval.interface_block)->set_location_range((yylsp[-1]), (yylsp[0]));
   }
#line 6673 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 2669 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.declarator_list) = (yyvsp[0].declarator_list);
      (yyvsp[0].declarator_list)->link.self_link();
   }
#line 6682 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 2674 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.declarator_list) = (yyvsp[-1].declarator_list);
      (yyvsp[0].declarator_list)->link.insert_before(& (yyval.declarator_list)->link);
   }
#line 6691 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 2682 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      void *ctx = state;
      ast_fully_specified_type *type = (yyvsp[-2].fully_specified_type);
      type->set_location((yylsp[-2]));

      if (type->qualifier.flags.q.attribute) {
         _mesa_glsl_error(& (yylsp[-2]), state,
                          "keyword 'attribute' cannot be used with "
                          "interface block member");
      } else if (type->qualifier.flags.q.varying) {
         _mesa_glsl_error(& (yylsp[-2]), state,
                          "keyword 'varying' cannot be used with "
                          "interface block member");
      }

      (yyval.declarator_list) = new(ctx) ast_declarator_list(type);
      (yyval.declarator_list)->set_location((yylsp[-1]));

      (yyval.declarator_list)->declarations.push_degenerate_list_at_head(& (yyvsp[-1].declaration)->link);
   }
#line 6716 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 2706 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if (!state->default_uniform_qualifier->merge_qualifier(& (yylsp[-2]), state, (yyvsp[-2].type_qualifier))) {
         YYERROR;
      }
      (yyval.node) = NULL;
   }
#line 6727 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 2714 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      (yyval.node) = NULL;
      if (!state->in_qualifier->merge_in_qualifier(& (yylsp[-2]), state, (yyvsp[-2].type_qualifier), (yyval.node))) {
         YYERROR;
      }
   }
#line 6738 "glsl_parser.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 2722 "../../../src/glsl/glsl_parser.yy" /* yacc.c:1646  */
    {
      if (state->stage != MESA_SHADER_GEOMETRY) {
         _mesa_glsl_error(& (yylsp[-2]), state,
                          "out layout qualifiers only valid in "
                          "geometry shaders");
      } else {
         if ((yyvsp[-2].type_qualifier).flags.q.prim_type) {
            /* Make sure this is a valid output primitive type. */
            switch ((yyvsp[-2].type_qualifier).prim_type) {
            case GL_POINTS:
            case GL_LINE_STRIP:
            case GL_TRIANGLE_STRIP:
               break;
            default:
               _mesa_glsl_error(&(yylsp[-2]), state, "invalid geometry shader output "
                                "primitive type");
               break;
            }
         }
         if (!state->out_qualifier->merge_qualifier(& (yylsp[-2]), state, (yyvsp[-2].type_qualifier)))
            YYERROR;

         /* Allow future assigments of global out's stream id value */
         state->out_qualifier->flags.q.explicit_stream = 0;
      }
      (yyval.node) = NULL;
   }
#line 6770 "glsl_parser.cpp" /* yacc.c:1646  */
    break;


#line 6774 "glsl_parser.cpp" /* yacc.c:1646  */
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
