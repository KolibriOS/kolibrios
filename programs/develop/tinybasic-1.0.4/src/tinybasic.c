/*
 * Tiny BASIC
 * Interpreter and Compiler Main Program
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 04-Aug-2019
 */


/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"
#include "errors.h"
#include "parser.h"
#include "statement.h"
#include "interpret.h"
#include "formatter.h"
#include "generatec.h"

#ifdef _KOLIBRI
    #include <sys/ksys.h>
    #define KTCC_BIN "/kolibrios/develop/tcc/tcc"
    #define KTCC_FLAGS "-nobss %s -o %s"
#endif

/* static variables */
static char *input_filename = NULL; /* name of the input file */
static enum { /* action to take with parsed program */
  OUTPUT_INTERPRET, /* interpret the program */
  OUTPUT_LST, /* output a formatted listing */
  OUTPUT_C, /* output a C program */
  OUTPUT_EXE /* output an executable */
} output = OUTPUT_INTERPRET;
static ErrorHandler *errors; /* universal error handler */
static LanguageOptions *loptions; /* language options */


/*
 * Level 2 Routines
 */


/*
 * Set line number option
 * params:
 *   char*   option   the option supplied on the command line
 */
static void set_line_numbers (char *option) {
  if (! strncmp ("optional", option, strlen (option)))
    loptions->set_line_numbers (loptions, LINE_NUMBERS_OPTIONAL);
  else if (! strncmp ("implied", option, strlen (option)))
    loptions->set_line_numbers (loptions, LINE_NUMBERS_IMPLIED);
  else if (! strncmp ("mandatory", option, strlen (option)))
    loptions->set_line_numbers (loptions, LINE_NUMBERS_MANDATORY);
  else
    errors->set_code (errors, E_BAD_COMMAND_LINE, 0, 0);
}

/*
 * Set line number limit
 * params:
 *   char*   option   the option supplied on the command line
 */
static void set_line_limit (char *option) {
  int limit; /* the limit contained in the option */
  if (sscanf (option, "%d", &limit))
    loptions->set_line_limit (loptions, limit);
  else
    errors->set_code (errors, E_BAD_COMMAND_LINE, 0, 0);
}

/*
 * Set comment option
 * params:
 *   char*   option   the option supplied on the command line
 */
static void set_comments (char *option) {
  if (! strncmp ("enabled", option, strlen (option)))
    loptions->set_comments (loptions, COMMENTS_ENABLED);
  else if (! strncmp ("disabled", option, strlen (option)))
    loptions->set_comments (loptions, COMMENTS_DISABLED);
  else
    errors->set_code (errors, E_BAD_COMMAND_LINE, 0, 0);
}

/*
 * Set the output options
 * params:
 *   char*   option   the option supplied on the command line
 */
static void set_output (char *option) {
  if (! strcmp ("lst", option))
    output = OUTPUT_LST;
  else if (! strcmp ("c", option))
    output = OUTPUT_C;
  else if (! strcmp ("exe", option))
    output = OUTPUT_EXE;
  else
    errors->set_code (errors, E_BAD_COMMAND_LINE, 0, 0);
}

/*
 * Set the GOSUB stack limit option
 * params:
 *   char*   option   the option supplied on the command line
 */
static void set_gosub_limit (char *option) {
  int limit; /* the limit contained in the option */
  if (sscanf (option, "%d", &limit))
    loptions->set_gosub_limit (loptions, limit);
  else
    errors->set_code (errors, E_BAD_COMMAND_LINE, 0, 0);
}


/*
 * Level 1 Routines
 */


/*
 * Process the command line options
 * params:
 *   int     argc   number of arguments on the command line
 *   char**  argv   the arguments
 */
static void set_options (int argc, char **argv) {

  /* local variables */
  int argn; /* argument number count */

  /* loop through all parameters */
  for (argn = 1; argn < argc && ! errors->get_code (errors); ++argn) {

    /* scan for line number options */
    if (! strncmp (argv[argn], "-n", 2))
      set_line_numbers (&argv[argn][2]);
    else if (! strncmp (argv[argn], "--line-numbers=", 15))
      set_line_numbers (&argv[argn][15]);

    /* scan for line number limit */
    else if (! strncmp (argv[argn], "-N", 2))
      set_line_limit (&argv[argn][2]);
    else if (! strncmp (argv[argn], "--line-limit=", 13))
      set_line_limit (&argv[argn][13]);

    /* scan for comment option */
    else if (! strncmp (argv[argn], "-o", 2))
      set_comments (&argv[argn][2]);
    else if (! strncmp (argv[argn], "--comments=", 11))
      set_comments (&argv[argn][11]);

    /* scan for output option */
    else if (! strncmp (argv[argn], "-O", 2))
      set_output (&argv[argn][2]);
    else if (! strncmp (argv[argn], "--output=", 9))
      set_output (&argv[argn][9]);

    /* scan for gosub stack limit */
    else if (! strncmp (argv[argn], "-g", 2))
      set_gosub_limit (&argv[argn][2]);
    else if (! strncmp (argv[argn], "--gosub-limit=", 14))
      set_gosub_limit (&argv[argn][14]);

    /* accept filename */
    else if (! input_filename)
      input_filename = argv[argn];

    /* raise an error upon illegal option */
    else
      errors->set_code (errors, E_BAD_COMMAND_LINE, 0, 0);
  }
}

/*
 * Output a formatted program listing
 * params:
 *   ProgramNode*   program   the program to output
 */
static void output_lst (ProgramNode *program) {

  /* local variables */
  FILE *output; /* the output file */
  char *output_filename; /* the output filename */
  Formatter *formatter; /* the formatter object */

  /* ascertain the output filename */
  output_filename = malloc (strlen (input_filename) + 5);
  if (output_filename) {

    /* open the output file */
    sprintf (output_filename, "%s.lst", input_filename);
    if ((output = fopen (output_filename, "w"))) {

      /* write to the output file */
      formatter = new_Formatter (errors);
      if (formatter) {
        formatter->generate (formatter, program);
        if (formatter->output)
          fprintf (output, "%s", formatter->output);
        formatter->destroy (formatter);
      }
      fclose (output);
    }

    /* deal with errors */
    else
      errors->set_code (errors, E_FILE_NOT_FOUND, 0, 0);

    /* free the output filename */
    free (output_filename);
  }

  /* deal with out of memory error */
  else
    errors->set_code (errors, E_MEMORY, 0, 0);
}

/*
 * Output a C source file
 * params:
 *   ProgramNode*   program   the parsed program
 */
static void output_c (ProgramNode *program) {

  /* local variables */
  FILE *output; /* the output file */
  char *output_filename; /* the output filename */
  CProgram *c_program; /* the C program */

  /* open the output file */
  output_filename = malloc (strlen (input_filename) + 5);
  sprintf (output_filename, "%s.c", input_filename);
  if ((output = fopen (output_filename, "w"))) {

    /* write to the output file */
    c_program = new_CProgram (errors, loptions);
    if (c_program) {
      c_program->generate (c_program, program);
      if (c_program->c_output)
        fprintf (output, "%s", c_program->c_output);
      c_program->destroy (c_program);
    }
    fclose (output);
  }

  /* deal with errors */
  else
    errors->set_code (errors, E_FILE_NOT_FOUND, 0, 0);

  /* clean up allocated memory */
  free (output_filename);
}

/*
 * Invoke a compiler to turn a C source file into an executable
 * params:
 *   char*   basic_filename   The BASIC program's name
 */
static void output_exe (char *command, char *basic_filename) {

  /* local variables */
  char
    c_filename[256], /* the name of the C source */
    exe_filename[256], /* the base name of the executable */
    final_command[1024], /* the constructed compiler command */
    *ext, /* position of extension character '.' in filename */
    *src, /* source pointer for string copying */
    *dst; /* destination pointer for string copying */

  /* work out the C and EXE filenames */
  sprintf (c_filename, "%s.c", basic_filename);
  strcpy (exe_filename, basic_filename);
  if ((ext = strchr (exe_filename, '.')))
    *ext = '\0';
  else
    strcat (exe_filename, ".out");

#ifndef _KOLIBRI
  /* build the compiler command */
  src = command;
  dst = final_command;
  while (*src) {
    if (! strncmp (src, "$(TARGET)", strlen ("$(TARGET)"))) {
      strcpy (dst, exe_filename);
      dst += strlen (exe_filename);
      src += strlen ("$(TARGET)");
    } else if (! strncmp (src, "$(SOURCE)", strlen ("$(SOURCE)"))) {
      strcpy (dst, c_filename);
      dst += strlen (c_filename);
      src += strlen ("$(SOURCE)");
    } else
      *(dst++) = *(src++);
  }
  *dst = '\0';

  /* run the compiler command */
  system (final_command);
#else
  sprintf(final_command, KTCC_FLAGS, c_filename, exe_filename);
  if(!_ksys_exec(KTCC_BIN, final_command)){
    printf("Bad command: %s %s\n", KTCC_BIN, final_command);
    exit(0);
  }
#endif
}

/*
 * Top Level Routine
 */


/*
 * Main Program
 * params:
 *   int     argc   number of arguments on the command line
 *   char**  argv   the arguments
 * returns:
 *   int            any error code from processing/running the program
 */
int main (int argc, char **argv) {
  /* local variables */
  FILE *input; /* input file */
  ProgramNode *program; /* the parsed program */
  ErrorCode code; /* error returned */
  Parser *parser; /* parser object */
  Interpreter *interpreter; /* interpreter object */
  char
    *error_text, /* error text message */
    *command; /* command for compilation */

  /* interpret the command line arguments */
  errors = new_ErrorHandler ();
  loptions = new_LanguageOptions ();
  set_options (argc, argv);

  /* give usage if filename not given */
  if (! input_filename) {
    printf ("Usage: tinybas [OPTIONS] INPUT-FILE\n");
    errors->destroy (errors);
    loptions->destroy (loptions);
    return 0;
  }
  /* otherwise attempt to open the file */
  if (!(input = fopen (input_filename, "r"))) {
    printf ("Error: cannot open file %s\n", input_filename);
    errors->destroy (errors);
    loptions->destroy (loptions);
    return E_FILE_NOT_FOUND;
  }

  /* get the parse tree */
  parser = new_Parser (errors, loptions, input);
  program = parser->parse (parser);
  parser->destroy (parser);
  fclose (input);

  /* deal with errors */
  if ((code = errors->get_code (errors))) {
    error_text = errors->get_text (errors);
    printf ("Parse error: %s\n", error_text);
    free (error_text);
    loptions->destroy (loptions);
    errors->destroy (errors);
    return code;
  }

  /* perform the desired action */
  switch (output) {
    case OUTPUT_INTERPRET:
      interpreter = new_Interpreter (errors, loptions);
      interpreter->interpret (interpreter, program);
      interpreter->destroy (interpreter);
      if ((code = errors->get_code (errors))) {
        error_text = errors->get_text (errors);
        printf ("Runtime error: %s\n", error_text);
        free (error_text);
      }
      break;
    case OUTPUT_LST:
      output_lst (program);
      break;
    case OUTPUT_C:
      output_c (program);
      break;
    case OUTPUT_EXE:
      
    #ifndef _KOLIBRI 
        if ((command = getenv ("TBEXE"))) {
            output_c (program);
            output_exe (command, input_filename);
        } else {
            printf ("TBEXE not set.\n");
            break;
        }
    #else
        output_c (program);
        output_exe (NULL, input_filename);
        break;
    #endif
  }
  /* clean up and return success */
  program_destroy (program);
  loptions->destroy (loptions);
  errors->destroy (errors);
  exit(0);
}
