// MKEXP
// Copyright (C) maxcodehack, 2021

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("usage: %s [symbols.txt exports.c]\n", argv[0]);
		return 0;
	}
	FILE *input, *output;
	if ((input = fopen(argv[1], "r")) == NULL)
	{
		printf("error: file \"%s\" not found\n", argv[1]);
		return 0;
	}
	char buf[10000];
	
	// Head
	strcpy(buf, \
			"#include <math.h>\n" \
			"#include <stdio.h>\n" \
			"#include <string.h>\n" \
			"#include <stdlib.h>\n" \
			"#include <time.h>\n" \
			"#include <sys/dirent.h>\n" \
			"#include <shell_api.h>\n" \
			"#include <ksys.h>\n\n" \
			"ksys_coff_etable_t EXPORTS[] = {\n");
	
	// Generate
	char symbol[256];
	while(fscanf(input, "%s", symbol) != EOF) {
		char temp[256];
		sprintf(temp, "{\"%s\", %s},\n", symbol, symbol);
		strcat(buf, temp);
	}
	strcat(buf, "NULL,\n};");
	fclose(input);
	
	// Output generated
	output = fopen(argv[2], "w");
	fputs(buf, output);
	fclose(output);
	
	printf("Done, check %s!\n", argv[2]);
	
	return 0;
}
