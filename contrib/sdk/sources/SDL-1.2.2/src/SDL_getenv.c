
/* Not all environments have a working getenv()/putenv() */

#ifdef TEST_MAIN
#define NEED_SDL_GETENV
#endif

#include "SDL_getenv.h"

#ifdef NEED_SDL_GETENV

#include <stdlib.h>
#include <string.h>

static char **SDL_env = (char **)0;

/* Put a variable of the form "name=value" into the environment */
int SDL_putenv(const char *variable)
{
	const char *name, *value;
	int added;
	int len, i;
	char **new_env;
	char *new_variable;

	/* A little error checking */
	if ( ! variable ) {
		return(-1);
	}
	name = variable;
	for ( value=variable; *value && (*value != '='); ++value ) {
		/* Keep looking for '=' */ ;
	}
	if ( *value ) {
		++value;
	} else {
		return(-1);
	}

	/* Allocate memory for the variable */
	new_variable = (char *)malloc(strlen(variable)+1);
	if ( ! new_variable ) {
		return(-1);
	}
	strcpy(new_variable, variable);

	/* Actually put it into the environment */
	added = 0;
	i = 0;
	if ( SDL_env ) {
		/* Check to see if it's already there... */
		len = (value - name);
		for ( ; SDL_env[i]; ++i ) {
			if ( strncmp(SDL_env[i], name, len) == 0 ) {
				break;
			}
		}
		/* If we found it, just replace the entry */
		if ( SDL_env[i] ) {
			free(SDL_env[i]);
			SDL_env[i] = new_variable;
			added = 1;
		}
	}

	/* Didn't find it in the environment, expand and add */
	if ( ! added ) {
		new_env = realloc(SDL_env, (i+2)*sizeof(char *));
		if ( new_env ) {
			SDL_env = new_env;
			SDL_env[i++] = new_variable;
			SDL_env[i++] = (char *)0;
			added = 1;
		} else {
			free(new_variable);
		}
	}
	return (added ? 0 : -1);
}

/* Retrieve a variable named "name" from the environment */
char *SDL_getenv(const char *name)
{
	int len, i;
	char *value;

	value = (char *)0;
	if ( SDL_env ) {
		len = strlen(name);
		for ( i=0; SDL_env[i] && !value; ++i ) {
			if ( (strncmp(SDL_env[i], name, len) == 0) &&
			     (SDL_env[i][len] == '=') ) {
				value = &SDL_env[i][len+1];
			}
		}
	}
	return value;
}

#endif /* NEED_GETENV */

#ifdef TEST_MAIN
#include <stdio.h>

int main(int argc, char *argv[])
{
	char *value;

	printf("Checking for non-existent variable... ");
	fflush(stdout);
	if ( ! getenv("EXISTS") ) {
		printf("okay\n");
	} else {
		printf("failed\n");
	}
	printf("Setting FIRST=VALUE1 in the environment... ");
	fflush(stdout);
	if ( putenv("FIRST=VALUE1") == 0 ) {
		printf("okay\n");
	} else {
		printf("failed\n");
	}
	printf("Getting FIRST from the environment... ");
	fflush(stdout);
	value = getenv("FIRST");
	if ( value && (strcmp(value, "VALUE1") == 0) ) {
		printf("okay\n");
	} else {
		printf("failed\n");
	}
	printf("Setting SECOND=VALUE2 in the environment... ");
	fflush(stdout);
	if ( putenv("SECOND=VALUE2") == 0 ) {
		printf("okay\n");
	} else {
		printf("failed\n");
	}
	printf("Getting SECOND from the environment... ");
	fflush(stdout);
	value = getenv("SECOND");
	if ( value && (strcmp(value, "VALUE2") == 0) ) {
		printf("okay\n");
	} else {
		printf("failed\n");
	}
	printf("Setting FIRST=NOVALUE in the environment... ");
	fflush(stdout);
	if ( putenv("FIRST=NOVALUE") == 0 ) {
		printf("okay\n");
	} else {
		printf("failed\n");
	}
	printf("Getting FIRST from the environment... ");
	fflush(stdout);
	value = getenv("FIRST");
	if ( value && (strcmp(value, "NOVALUE") == 0) ) {
		printf("okay\n");
	} else {
		printf("failed\n");
	}
	printf("Checking for non-existent variable... ");
	fflush(stdout);
	if ( ! getenv("EXISTS") ) {
		printf("okay\n");
	} else {
		printf("failed\n");
	}
	return(0);
}
#endif /* TEST_MAIN */

