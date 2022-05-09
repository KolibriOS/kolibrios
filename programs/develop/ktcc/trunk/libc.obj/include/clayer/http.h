/*
    This is adapded thunk for http.obj sys library
    .h is equal to svn:\\programs\develop\libraries\http\http_en.txt 

    Adapted for TCC's dynamic API by Magomed Kostoev, 2020
*/

#ifndef KOLIBRI_HTTP_H
#define KOLIBRI_HTTP_H

#include <stddef.h>

// Bitflags for http_msg.flags
// status

#define HTTP_FLAG_HTTP11             1 << 0
#define HTTP_FLAG_GOT_HEADER         1 << 1
#define HTTP_FLAG_GOT_ALL_DATA       1 << 2
#define HTTP_FLAG_CONTENT_LENGTH     1 << 3
#define HTTP_FLAG_CHUNKED            1 << 4
#define HTTP_FLAG_CONNECTED          1 << 5

// user options
#define HTTP_FLAG_KEEPALIVE          1 << 8
#define HTTP_FLAG_STREAM             1 << 9
#define HTTP_FLAG_REUSE_BUFFER       1 << 10
#define HTTP_FLAG_BLOCK              1 << 11

// error
#define HTTP_FLAG_INVALID_HEADER     1 << 16
#define HTTP_FLAG_NO_RAM             1 << 17
#define HTTP_FLAG_SOCKET_ERROR       1 << 18
#define HTTP_FLAG_TIMEOUT_ERROR      1 << 19
#define HTTP_FLAG_TRANSFER_FAILED    1 << 20

/*
User flags:

For the flag codes themselves, see http.inc file.

 FLAG_KEEPALIVE will keep the connection open after first GET/POST/.. so you can send a second request on the same TCP session. 
In this case, the session must be closed manually when done by using the exported disconnect() function.

 FLAG_STREAM will force receive() to put the received content in a series of fixed size buffers, instead of everything in one big buffer.
This can be used for example to receive an internet radio stream, 
but also to download larger files for which it does not make sense to put them completely in RAM first.

 FLAG_REUSE_BUFFER is to be used in combination with FLAG_STREAM and will make receive() function re-use the same buffer.
This, for example, can be used when downloading a file straight to disk.

 FLAG_BLOCK will make receive() function blocking. This is only to be used when receiving one file from a thread that has no other work.
If however, you want to receive multiple files, or do other things in the program mainloop, you should call the receive function periodically. 
You may use system function 10 or 23 to wait for network event before calling one or more receive() functions. 
*/

#pragma pack(push,1)
typedef struct http_msg_s {
    unsigned socket;           // socket on which the actual transfer happens
    unsigned flags;            // flags, reflects status of the transfer using bitflags
    unsigned write_ptr;        // internal use only (where to write new data in buffer)
    unsigned buffer_length;    // internal use only (number of available bytes in buffer)
    unsigned chunk_ptr;        // internal use only (where the next chunk begins)
    unsigned timestamp;        // internal use only (when last data was received)
    unsigned status;           // HTTP status
    unsigned header_length;    // length of HTTP header
    void *   content_ptr;      // ptr to content
    unsigned content_length;   // total length of HTTP content
    unsigned content_received; // number of currently received content bytes
    char *   http_header;
} http_msg;
#pragma pack(pop)

/*
    url = pointer to ASCIIZ URL
    identifier = identifier of previously opened connection (keep-alive), or 0 to open a new one.
    flags = bit flags (see end of this document).
    add_header = pointer to ASCIIZ additional header parameters, or null for none.
    Every additional parameter must end with CR LF bytes, including the last line.
    Initiates a HTTP connection, using 'GET' method.
    Returns NULL on error, identifier otherwise.
*/
DLLAPI http_msg * __stdcall http_get(const char *url, http_msg *identifier, unsigned flags, const char *add_header);

/*
    url = pointer to ASCIIZ URL
    identifier = identifier of previously opened connection (keep-alive), or 0 to open a new one.
    flags = bit flags (see end of this document).
    add_header = pointer to ASCIIZ additional header parameters, or null for none.    
    Every additional parameter must end with CR LF bytes, including the last line.    
    Initiate a HTTP connection, using 'HEAD' method.
    Returns NULL on error, identifier otherwise.
*/
DLLAPI http_msg * __stdcall http_head(const char *url, http_msg *identifier, unsigned flags, const char *add_header);

/*
    url = pointer to ASCIIZ URL
    identifier = identifier of previously opened connection (keep-alive), or 0 to open a new one.
    flags = bit flags (see end of this document).
    add_header = pointer to ASCIIZ additional header parameters, or null for none.    
    Every additional parameter must end with CR LF bytes, including the last line.
    content-type = pointer to ASCIIZ string containing content type.
    content-length = length of the content (in bytes).
    Initiate a HTTP connection, using 'POST' method.
    The content itself must be send to the socket (which you can find in the structure),
    using system function 75, 6.
    Returns 0 on error, identifier otherwise
*/
DLLAPI http_msg * __stdcall http_post(const char *url, http_msg *identifier, unsigned flags, const char *add_header,
                                  const char *content_type, unsigned content_length);

/*
    identifier = identifier which one of the previous functions returned
    This procedure will handle all incoming data for a connection and place it in the buffer.
    As long as the procedure expects more data, -1 is returned and the procedure must be called again.
    When transfer is done, the procedure will return 0. 
    The receive procedure is non-blocking by default, but can be made to block by setting FLAG_BLOCK.

    The HTTP header is placed together with some flags and other attributes in the http_msg structure.
    This structure is defined in http.inc (and not copied here because it might still change.)
    The identifier used by the functions is actually a pointer to this structure.
    In the dword named .flags, the library will set various bit-flags indicating the status of the process.
    (When a transfer is done, one should check these bit-flags to find out if the transfer was error-free.)
    The HTTP header is placed at the end of this structure. The content is placed in another buffer.
    The dword .status contains the status code received from the server (e.g. 200 for OK).
    In header_length you'll find the length of the header as soon as it has been received.
    In content_ptr you'll find a pointer to the actual content.
    In content_length you'll find the length of the content. 
    In content_received, you'll find the number of content bytes already received.
*/
DLLAPI int __stdcall http_receive(http_msg *identifier);

/*
    identifier = identifier which one of the previous functions returned
    dataptr = pointer to the data you want to send
    datalength = length of the data to send (in bytes)
    This procedure can be used to send data to the server (POST)
    Returns number of bytes sent, -1 on error
*/
DLLAPI int __stdcall http_send(http_msg *identifier, void *dataptr, unsigned datalength);

/*
    Sometimes the http_receive function receives incomplete data. If you have the same problem then a macro can help you:
*/

DLLAPI int __stdcall http_free(http_msg *identifier);
/*
    Free unused data 
*/

#define http_long_receive(x) while(http_receive(x)){}; 

#endif // KOLIBRI_HTTP_H
