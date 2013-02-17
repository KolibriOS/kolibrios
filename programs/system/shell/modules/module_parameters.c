
/// ===========================================================

int parameters_prepare(char *param, char* argv[])
{

int i, param_len;
int argc = 1;
int quote = 0;
int argv_len;

param_len = strlen(param);
if ( param_len == 0 )
   return 0;

argv[0] = (char*) malloc(4096);

argv_len = 0;
for (i = 0; i < param_len; i++)
    {
    switch (param[i])
           {
           case '"':
                if (quote == 0)
                    quote = 1;
                else
                    {
                    quote = 0;
                    argv[argc-1][argv_len] = '\0';
                    argc++;
                    argv[argc-1] = (char*) malloc(4096);
                    argv[argc-1][argv_len] = '\0';
                    argv_len = 0;
                    }
                break;

           case 9:
           case 32:
                if (quote == 0)
                   {
                   if ( ( param[i+1] != 32) || ( param[i+1] != 9) )
                      {
                      if (argv_len != 0)
                         {
                         argv[argc-1][argv_len] = '\0';
                         argc++;
                         argv[argc-1] = (char*) malloc(4096);
                         argv[argc-1][argv_len] = '\0';
                         argv_len = 0;
                         }
                      }
                   }
                   else
                    {
                    argv[argc-1][argv_len] = param[i];
                    argv_len++;
                    }

                   break;

           default:
                   argv[argc-1][argv_len] = param[i];
                   argv_len++;
                   break;
           };
    }

argv[argc-1][argv_len] = '\0';

if ( strlen(argv[argc-1]) == 0 )
   {
   free(argv[argc-1]);
   argc--;
   }

return argc;
}

/// ===========================================================

void parameters_free(int argc, char* argv[])
{

int i;

for (i = 0; i < argc; i++)
    free(argv[i]);

}

/// ===========================================================

