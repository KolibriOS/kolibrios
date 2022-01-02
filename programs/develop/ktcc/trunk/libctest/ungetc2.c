/* ungetc example */
#include <stdio.h>

void trace_file(FILE* f, char* cmt);

int main ()
{
  FILE * pFile;
  int c;
  char buffer [256];

  pFile = fopen ("myfile.txt","rt");
  if (pFile==NULL) perror ("Error opening file");
  else while (!feof (pFile)) {
	trace_file(pFile, "1");	

    c=getc (pFile);

	trace_file(pFile, "before ungetc");	

    if (c == EOF) break;
    if (c == '#') ungetc ('@',pFile);
    else ungetc (c,pFile);
	
	trace_file(pFile, "after");	

    if (fgets (buffer,255,pFile) != NULL)
      puts (buffer);
    else break;
  }
  return 0;
}

void trace_file(FILE* f, char* cmt)
{
	printf("%s[%s]\n", cmt, f->buffer);
	printf("mode=%0X, filesize=%d, filepos=%d\n", f->mode, f->filesize, f->filepos);
	printf("ungetc=%d, buffer_start=%d, buffer_end=%d\n", f->ungetc_buf, f->buffer_start, f->buffer_end);	
}