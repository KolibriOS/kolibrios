// export_table_gen
// Copyright (C) maxcodehack and turbocat2001, 2021

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: %s <symbols.txt> <exports.c>\n", argv[0]);
		return 0;
	}
	FILE *input, *output;
	if ((input = fopen(argv[1], "r")) == NULL)
	{
		printf("error: file \"%s\" not found\n", argv[1]);
		return 1;
	}
	char buf[10000];
	// Head
	strcpy(buf, \
			"#include <stdio.h>\n" \
			"#include <string.h>\n" \
			"#include <stdlib.h>\n" \
			"#include <time.h>\n" \
			"#include <sys/dirent.h>\n" \
			"#include <sys/ksys.h>\n\n" \
			"#include <math.h>\n\n" \
			"#include <setjmp.h>\n\n" \
			"ksys_coff_etable_t EXPORTS[] = {\n");
	
	// Generate
	char symbol[256];
	while(fscanf(input, "%s", symbol) != EOF) {
		if(symbol[0]!='!'){
			char temp[256];
			sprintf(temp, "{\"%s\", &%s},\n", symbol, symbol);
			strcat(buf, temp);
		}
	}
	strcat(buf, "NULL,\n};");
	fclose(input);
	
	// Output generated
	output = fopen(argv[2], "w");
	if (output == NULL)
	{
		printf("Unable to write to file: '%s'!\n", argv[2]);
		return 1;
	}

	fputs(buf, output);
	fclose(output);
	
	printf("Done, check %s!\n", argv[2]);
	
	return 0;
}
