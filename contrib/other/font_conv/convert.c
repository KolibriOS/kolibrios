/*
    convert.c: converts text bitmaps into binary bitmaps.
    This has been written with an aim to detect any errors that
    might have crept in while generating a text bitmap for a font.

    Copyright 2011 dunkaist <dunkaist@gmail.com>
    Copyright 2014 ashmew2 <ashmew2@gmail.com>
    Distributed under the terms of the GNU General Public License v3.
    See http://www.gnu.org/licenses/gpl.txt for the full license text.
*/

/*
    For TRANSLATION, only the hard coded strings such as in usage[] need to
    rewritten in the desired language. The translation selection should be
    inside an #if LANG == RUS {} #else {} block. All messsages are printed
    using msg_printx() and the translations need to be done for all such
    calls.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>

#define FONT_HEIGHT         9   /* The height of each symbol in input file */
#define FONT_WIDTH_MONO     5   /* Fixed width for mono fonts */
#define FONT_WIDTH_VAR      7   /* Max symbol width */
#define LINE_BUFFER_SIZE   12   /* For FONT_WIDTH_VAR and delimiters */
#define NUM_SYMBOLS       256   /* Number of symbols in input file */

enum error_code {
  ERROR_PARSING_ARGS = 1,
  ERROR_OPENING_FILE,
  ERROR_INVALID_INPUT_FILE
};

void msg_printx(int errnum, char *message, ...)
{
  va_list args;

  va_start(args, message);
  vfprintf(stderr, message, args);
  va_end(args);

  exit(errnum);
}

void usage_printx(char *prog_name)
{
  char usage[] = "Usage: \n"
                 "\t%s <INPUTFILE> <OUTPUTFILE> <-m>\n\n"
                 "Converts font bitmaps from text to binary format.\n\n"
                 "\tINPUTFILE\t The input file containing text bitmap.\n"
                 "\tOUTPUTFILE\t The output file for writing binary bitmap.\n"
                 "\t-m\t         Mono flag. -0 for mono. -1 otherwise.\n\n"
                 "Examples: \n"
                 "\t%s char.txt CHAR.MT -0\n"
                 "\t%s char2.txt CHAR2.MT -1\n\n"
                 "Exit status: \n"
                 "\t 0: Successful conversion.\n"
                 "\t 1: Incorrect arguments supplied to the command.\n"
                 "\t 2: Files could not be opened for read/write.\n"
                 "\t 3: Error(s) reading data from file.\n\n"
                 "This is a part of the font_conv program in KolibriOS.\n";

  msg_printx(ERROR_PARSING_ARGS, usage, prog_name, prog_name, prog_name);
}

char* get_row(bool use_mono)
{
  static int line_number = 1; /* Initialize to 1 for first line being read */
  static size_t len_line;
  static char line[LINE_BUFFER_SIZE];

  size_t len;
  char *current_line;
  int i;

  current_line = fgets(line, LINE_BUFFER_SIZE - 1, stdin);

  if(!current_line)
    msg_printx(ERROR_INVALID_INPUT_FILE, "Error: could not read line from input file.\n");

  len = strlen(current_line);

  if(len > 2)
    {
      if(line[len - 1] == '\n' && line[len - 2] == '\r')
	len -= 2;
      else if(line[len - 1] == '\n')
	--len;
      else if(line_number != ((FONT_HEIGHT + 1) * NUM_SYMBOLS)) /* if last line of input */
	msg_printx(ERROR_INVALID_INPUT_FILE, "Error: line %d: no newline character found in first %d bytes.\n", 12 - 1, line_number);

      line[--len] = '\0';
    }
  else
    msg_printx(ERROR_INVALID_INPUT_FILE, "Error: line %d: line too short.\n", line_number);

  if(line_number == 1) /* Processing first line in input */
    {
      if(len > FONT_WIDTH_VAR)
	msg_printx(ERROR_INVALID_INPUT_FILE, "Error: line %d: length of line is larger than %d (maximum allowed width).\n", line_number, FONT_WIDTH_VAR);

      if((use_mono) && (len != FONT_WIDTH_MONO))
	msg_printx(ERROR_INVALID_INPUT_FILE, "Error: line %d: length of line is not equal to %d (mono font width).\n", line_number, FONT_WIDTH_MONO);

      len_line = len;
    }
  else
    {
      if(len != len_line)
	msg_printx(ERROR_INVALID_INPUT_FILE, "Error: line %d: length of line does not match length of first line in file.\n", line_number);

      /*validate Row*/
      for(i = 0; line[i]; i++)
	if(!isprint(line[i]))
	  msg_printx(ERROR_INVALID_INPUT_FILE, "Error: line %d: non printable characters found on line.\n", line_number);      
    }

  ++line_number;

  return line;
}

int do_symbol(short int font_width)
{
  short int row, col;
  int data;

  for(row = FONT_HEIGHT; row; row--)
    {
      char *line = get_row(font_width == FONT_WIDTH_MONO);

      data = 0; /* Create empty byte for storing line */

      for(col = 0; col < font_width; col++)
        {
	  if(line[col] != ' ')
	    data |= 1<<col; /* Get corresponding bit for non-space character */
        }

      putchar(data);
    }
  return 0;
}

int main(int argc, char *argv[])
{
  char *input_file = NULL;
  char *output_file = NULL;
  short int char_num;
  bool use_mono;

  if(argc != 4) /* Required argc is 4, for three mandatory arguments. */
    usage_printx(argv[0]);
  else
    {
      if(!strcmp(argv[3],"-0"))
	use_mono = true;
      else if(!strcmp(argv[3],"-1"))
	use_mono = false;
      else
	usage_printx(argv[0]);
    }

  input_file = argv[1];
  output_file = argv[2];

  if(!freopen(input_file, "rt", stdin))
    msg_printx(ERROR_OPENING_FILE, "Error: unable to open %s for reading.\n", input_file);  

  if(!freopen(output_file, "wb", stdout))
    msg_printx(ERROR_OPENING_FILE, "Error: unable to open %s for writing.\n", output_file);  

  for(char_num = NUM_SYMBOLS; char_num; char_num--)
    {
      char *line = get_row(use_mono);

      if(use_mono)
	{
	  do_symbol(FONT_WIDTH_MONO);
	}
      else
	{
	  size_t len = strlen(line);

	  int p = line[len - 1];

	  putchar(p == ' '? 0x08 : p-47); /* Put a backspace character or a decimal digit */
	  do_symbol(FONT_WIDTH_VAR);
	}
    }

  return 0;
}
