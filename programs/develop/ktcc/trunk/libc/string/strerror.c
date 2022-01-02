#include <string.h>
#include <errno.h>

char* strerror(int err)
{
    char *msg;
    switch(err)
    {
    case E_SUCCESS:
        msg = "Success";
        break;
    case -1:
        msg = "End of file";
        break;
    case E_UNSUPPORTED:
        msg = "Function is not supported for the given file system";
        break;
    case E_UNKNOWNFS:
        msg = "Unknown file system";
        break;
    case E_NOTFOUND:
        msg = "File not found";
        break;
    case E_EOF:
        msg = "End of file, EOF";
        break;
    case E_INVALIDPTR:
        msg = "Pointer lies outside of application memory";
        break;
    case E_DISKFULL:
        msg = "Disk is full";
        break;
    case E_FSYSERROR:
        msg = "Dile system error";
        break;
    case E_ACCESS:
        msg = "Access denied";
        break;
    case E_HARDWARE:
        msg = "Device error";
        break;
    case E_NOMEM:
        msg = "File system requires more memory";
        break;
    case E_NOMEM2:
        msg = "Not enough memory";
        break;
    case E_FILEFMT:
        msg = "File is not executable";
        break;
    case E_TOOMANY:
        msg = "Too many processes";
        break;
    /* Socket errors */
    case ENOBUFS:
        msg = "Broken buffer";
        break;
    case EINPROGRESS:
        msg = "Operation now in progress";
        break;
    case EOPNOTSUPP:
        msg = "Operation not supported on transport endpoint";
        break;
    case EWOULDBLOCK:
        msg = "Operation would block";
        break;
    case ENOTCONN:
        msg = "Transport endpoint is not connected";
        break;
    case EALREADY: 
        msg = "Operation already in progress";
        break;
    case EINVALUE: 
        msg = "Invalid argument"; 
        break; 
    case EMSGSIZE:
        msg = "Message too long";
        break;
    case ENOMEM:
        msg = "Out of memory";
        break;
    case EADDRINUSE:   
        msg = "Address already in use"; 
        break;
    case ECONNREFUSED:
        msg = "Connection refused";
        break;
    case ECONNRESET:
        msg = "Connection reset by peer";
        break;
    case EISCONN:
        msg = "Transport endpoint is already connected"; 
        break;
    case ETIMEDOUT:
        msg = "Connection timed out";
        break;
    case ECONNABORTED: 
        msg = "Software caused connection abort";
        break;
    default:
        msg = "Unknown error";
        break;
    }
    return msg;
}
