#define MEMSIZE 1024*100

#include "../lib/kolibri.h"
#include "../lib/collection.h"
#include "../lib/gui.h"
#include "../lib/fs.h"

#include "../lib/obj/http.h"
#include "../lib/obj/console.h"

collection links;
#include "urls.h"

_http http;
char accept_language[]= "Accept-Language: en\n";

int url_id=0;

void main()
{
	char savepath[100];
	load_dll(libHTTP, #http_lib_init,1);
	load_dll(libConsole, #con_init, 0);

	con_init stdcall (70, 40, 70, 1020, "Web stability test");
	urls_add();
	con_write_asciiz stdcall ("Redirect is not handled yet.\n");
	con_write_asciiz stdcall ("All pages are saved into /tmp0/1\n\n");
	con_write_asciiz stdcall ("Downloading pages...\n\n");
	pause(100);
	get_next_url();

	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER + EVM_STACK);
	loop() switch(@WaitEventTimeout(200))
	{
	case evNetwork:	
		if (!http.receive_result) {
				sprintf(#savepath, "/tmp0/1/%s.htm", links.get(url_id)+7);
				CreateFile(http.content_received, http.content_pointer, #savepath);
				free(http.content_pointer);
				http_free stdcall (http.transfer);
				http.transfer=0;
				con_write_asciiz stdcall ("\n");
				get_next_url();
		}		
		if (http.transfer) {
			http.receive();	
		} else {
			con_write_asciiz stdcall (" => FAILED\n");
			get_next_url();
		}
	}
}

void get_next_url()
{
	char get_url[2500];
	dword url;

	url_id++;
	url = links.get(url_id);

	if (!url) {
		con_write_asciiz stdcall ("Download complete.");
		con_exit stdcall (0);
		ExitProcess();
	} else {
		con_write_asciiz stdcall (itoa(url_id));
		con_write_asciiz stdcall (". ");
		con_write_asciiz stdcall (links.get(url_id));

		if (!strncmp(url,"https:",6)) {
			sprintf(#get_url, "http://gate.aspero.pro/?site=%s", url);
		} else if (!strncmp(url,"http:",5)) {
			sprintf(#get_url, "%s", url);
		} else {
			get_next_url();
		}

		http.get(#get_url);
	}
}
