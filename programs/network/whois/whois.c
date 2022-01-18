/*
WHOIS port for KolibriOS (Adapted by turbocat2001).
The main code is taken from the site:
https://www.binarytides.com/whois-client-code-in-c-with-linux-sockets/
*/

#include <errno.h>
#include <sys/ksys.h>
#include <stdio.h>
#include <string.h>	
#include <stdlib.h>	
#include <sys/socket.h>	
#include <clayer/network.h>
#include <conio.h>

FILE *out=stdout;

#ifdef DEBUG
FILE *out=stderr;
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void show_help()
{
	puts("Usage: whois <host> [-f <file>]\n");
	puts(" host         Connect to server host");
	puts(" -f file      Redirecting output to file\n");
	puts("Example: whois google.com -f my.txt");
}

int get_whois_data(char * , char **);
int hostname_to_ip(char * , char *);
int whois_query(char * , char * , char **);
char *str_replace(char *search , char *replace , char *subject );
char* str_copy(char*);


int main(int argc , char *argv[])
{	
	char *domain , *data = NULL;
	int f_flag=0;

	if(argc==2){
		domain=strdup(argv[1]);
	}

	else if(!strcmp(argv[2], "-f") && argc==4){
		domain=strdup(argv[1]);
		if((out=fopen(argv[3],"w"))==NULL){
			printf("Error writing to file: '%s' !\n", argv[3]);
			exit(0);
		}
	}else{
		show_help();
		exit(0);
	}
	if(out==stdout){
		(*con_init_opt)(-1,-1,-1,-1, "Whois");
	}
	get_whois_data(domain , &data);
	exit(0);
}

/*
    Get the whois data of a domain
*/

int get_whois_data(char *domain , char **data)
{
	char ext[1024] , *pch , *response = NULL , *response_2 = NULL , *wch , *dt;

	//remove "http://" and "www."
	domain = str_replace("http://" , "" , domain);
	domain = str_replace("www." , "" , domain);

	//get the extension , com , org , edu
	dt = strdup(domain);

	if(dt == NULL){
		fprintf(out, "strdup failed");
	}
	pch = (char*)strtok(dt , ".");
	while(pch != NULL){
		strcpy(ext , pch);
		pch = strtok(NULL , ".");
	}
	// This will tell the whois server for the particular TLD like com , org
	if( whois_query("whois.iana.org" , ext , &response) == EXIT_FAILURE){
		fprintf(out, "Whois query failed");
        return 1;
	}
	fprintf(out, "\n\nResponse is:\n\n");
	fprintf(out, "%s", response);

	// Now analysze the response
	pch = strtok(response , "\n");
	while(pch != NULL){
		// Check if whois line
		wch = strstr(pch , "whois.");
		if(wch != NULL){
			break;
		}

		// Next line please
		pch = strtok(NULL , "\n");
	}
	// Now we have the TLD whois server in wch , query again
	//This will provide minimal whois information along with the parent whois server of the specific domain :)
	wch = strdup(wch);
	free(response);
    //This should not be necessary , but segmentation fault without this , why ?
	response = NULL;
	if(wch != NULL){
		fprintf(out,"\nTLD Whois server is : %s" , wch);
		if( whois_query(wch , domain , &response) == EXIT_FAILURE){
			fprintf(out, "Whois query failed\n");
            return EXIT_FAILURE;
		}
	}else{
		fprintf(out, "\nTLD whois server for %s not found\n" , ext);
		return EXIT_SUCCESS;
	}
	
	response_2 = strdup(response);
	
    // Again search for a whois server in this response. :)
	pch = strtok(response , "\n");
	while(pch != NULL){
		// Check if whois line
		wch = strstr(pch , "whois.");
		if(wch != NULL){
			break;
		}
		//Next line please
		pch = strtok(NULL , "\n");
	}

	/*
        If a registrar whois server is found then query it
    */
	if(wch){
		// Now we have the registrar whois server , this has the direct full information of the particular domain
		// so lets query again
		
		fprintf(out, "\nRegistrar Whois server is : %s" , wch);
		
		if( whois_query(wch , domain , &response) == EXIT_FAILURE ){
			fprintf(out, "Whois query failed");
		}else{
            fprintf(out, "\n%s" , response);
        }
	}
	/*
        otherwise echo the output from the previous whois result
    */
	else{
		fprintf(out, "%s" , response_2);
	}	
	return 0;
}

/*
    Perform a whois query to a server and record the response
*/
int whois_query(char *server , char *query , char **response)
{
	char ip[32] , message[100] , buffer[1500];
	int sock , read_size , total_size = 0;
	int WHOIS_PORT = 43;
    struct sockaddr dest;
    
	sock = socket(AF_INET4 , SOCK_STREAM , IPPROTO_TCP);
     
    //Prepare connection structures :)
    memset(&dest , 0 , sizeof(dest) );
	dest.sin_family = AF_INET;
    server = str_copy(server);
	
    server[strcspn(server, "\r\n")] = '\0';
	fprintf(out, "\nResolving: %s ...\n" , server);
	if(hostname_to_ip(server , ip) == EXIT_FAILURE ){
        fprintf(out, "Failed\n");
        return EXIT_FAILURE;
	}
	
	fprintf(out, "Found ip: %s \n" , ip);    
    dest.sin_addr = inet_addr(ip);
	dest.sin_port = PORT(WHOIS_PORT);

;	//Now connect to remote server
	if(connect(sock , (const struct sockaddr*) &dest , sizeof(dest)) < 0){
		perror("connect failed");
		perror(strerror(errno));
        return EXIT_FAILURE;
	}

	//Now send some data or message
	fprintf(out, "\nQuerying for: %s ...\n" , query);
	sprintf(message , "%s\r\n" , query);
	if( send(sock , message , strlen(message) , 0) < 0){
		perror("send failed");
        return EXIT_FAILURE;
	}
	
	//Now receive the response
	while((read_size = recv(sock, buffer, sizeof(buffer), 0))){
		*response = realloc(*response , read_size + total_size);
		if(*response == NULL){
			fprintf(out, "realloc failed");
            return EXIT_FAILURE;
		}
		memcpy(*response + total_size , buffer , read_size);
		total_size += read_size;
	}
	
	fprintf(out, "Done\n");
	
	*response = realloc(*response , total_size + 1);
	*(*response + total_size) = '\0';
	close(sock);
    return EXIT_SUCCESS;
}
/*
    Get the ip address of a given hostname
*/
int hostname_to_ip(char *hostname , char *ip)
{
	struct addrinfo *addr_info;
    char port_str[16]; sprintf(port_str, "%d", 80);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6 doesnt matter
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    if (getaddrinfo(hostname, port_str, 0, &addr_info) != 0) {
        freeaddrinfo(addr_info);
		return EXIT_FAILURE;
    }else{
		strcpy(ip, inet_ntoa(addr_info->ai_addr->sin_addr));
		return EXIT_SUCCESS;
	}
}
/*
    Search and replace a string with another string , in a string
*/
char *str_replace(char *search , char *replace , char *subject)
{
	char  *p = NULL , *old = NULL , *new_subject = NULL ;
	int c = 0 , search_size;
	
	search_size = strlen(search);
	
	//Count how many occurences
	for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search)){
		c++;
	}
	//Final size
	c = ( strlen(replace) - search_size )*c + strlen(subject);
	
	//New subject with new size
	new_subject = malloc( c );
	
	//Set it to blank
	strcpy(new_subject , "");
	
	//The start position
	old = subject;
	
	for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search)){
		//move ahead and copy some text from original subject , from a certain position
		strncpy(new_subject + strlen(new_subject) , old , p - old);
		
		//move ahead and copy the replacement text
		strcpy(new_subject + strlen(new_subject) , replace);
		
		//The new start position after this search match
		old = p + search_size;
	}
	
	//Copy the part after the last search match
	strcpy(new_subject + strlen(new_subject) , old);
	
	return new_subject;
}

char* str_copy(char *source)
{
    char *copy = malloc(strlen(source) + 1); 
    
    if(copy){
        strcpy(copy, source);
    }
    return copy;
} 
