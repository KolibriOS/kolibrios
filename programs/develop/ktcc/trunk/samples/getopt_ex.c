#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

void main(int argc, char *argv[]) {
    int c;
    if(argc<2)
    {
        puts("Usage: getopt_ex [options]\n");
        puts("-a        Show 'Option a'");
        puts("-B        Show 'Option B'");
        puts("-n [num]  Show 'num'");
    }
    while ((c = getopt(argc, argv, "aBn:")) != EOF) {
        switch (c) {
             case 'a':
                 puts("Option 'a'");
                 break;

             case 'B':
                 puts("Option 'B'");
                 break;

             case 'n':
                 printf("Option n: value=%d\n", atoi(optarg));
                 break;

             case '?':
                 printf("ERROR: illegal option %s\n", argv[optind-1]);
                 exit(0);

             default:
                 printf("WARNING: no handler for option %c\n", c);
                 exit(0);
         }
     }
     exit(0);
 } 
