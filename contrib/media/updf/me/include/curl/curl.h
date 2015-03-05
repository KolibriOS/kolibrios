typedef int CURL;
typedef int CURLM;

#include <time.h>

char *curl_unescape( char * url , int length );
void curl_free( char * ptr );
char *curl_easy_unescape( CURL * curl , char * url , int inlength , int * outlength );
void curl_easy_cleanup(CURL * handle );
CURL *curl_easy_init( );
time_t curl_getdate(char * datestring , time_t *now );
