#include <curl/curl.h>
#include <menuet/os.h>

char *curl_unescape( char * url , int length ) {
	
	__menuet__debug_out("CURL:unescape\n");
	return url;
	
}

void curl_free( char * ptr ){
	__menuet__debug_out("CURL:free?\n");
}

char *curl_easy_unescape( CURL * curl , char * url , int inlength , int * outlength ){
	__menuet__debug_out("CURL:easyunescape!\n");
	*outlength=inlength;
	return url;
	
}

void curl_easy_cleanup(CURL * handle ) {
	__menuet__debug_out("CURL:cleanup\n");
}

CURL *curl_easy_init( ){
	__menuet__debug_out("CURL:init\n");
}

time_t curl_getdate(char * datestring , time_t *now ){
	__menuet__debug_out("CURL:getdate\n");
	return 0;
	
}


