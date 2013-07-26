//Network library

dword netcode_lib = #a_netcode_lib;
char a_netcode_lib[21]="/sys/lib/netcode.obj\0";

dword base64_encode     = #aBase64_encode;
dword base64_decode     = #aBase64_decode;
dword qp_decode         = #aQp_decode;
$DD 2 dup 0

char aBase64_encode[14] = "base64_encode\0";
char aBase64_decode[14] = "base64_decode\0";
char aQp_decode[10]     = "qp_decode\0";

/*int base64_encode(char inp[], char outp[], int len);
Кодирование массива inp длиной len в массив outp (строку с '\0'). Функция возвращает длину outp.

int base64_decode(char inp[], char outp[], int len);
Декодирование массива inp длиной len в массив outp (строку с '\0'). Функция возвращает длину outp.*/