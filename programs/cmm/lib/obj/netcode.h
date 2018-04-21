//Network library
#ifndef INCLUDE_NETCODE_H
#define INCLUDE_NETCODE_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif
dword netcode_lib = #a_netcode_lib;
char a_netcode_lib[]="/sys/lib/netcode.obj";

dword base64_encode     = #aBase64_encode;
dword base64_decode     = #aBase64_decode;
dword qp_decode         = #aQp_decode;
$DD 2 dup 0

char aBase64_encode[] = "base64_encode";
char aBase64_decode[] = "base64_decode";
char aQp_decode[]     = "qp_decode";

/*int base64_encode(char inp[], char outp[], int len);
Кодирование массива inp длиной len в массив outp (строку с '\0'). Функция возвращает длину outp.

int base64_decode(char inp[], char outp[], int len);
Декодирование массива inp длиной len в массив outp (строку с '\0'). Функция возвращает длину outp.*/

#endif