#include <string.h>

char* strerror(int err)
{
    char *msg;
    switch(err)
    {
    case 0:
        msg = "success";
        break;
    case -1:
        msg = "end of file";
        break;
    case -2:
        msg = "function is not supported for the given file system";
        break;
    case -3:
        msg = "unknown file system";
        break;
    case -5:
        msg = "file not found";
        break;
    case -6:
        msg = "end of file, EOF";
        break;
    case -7:
        msg = "pointer lies outside of application memory";
        break;
    case -8:
        msg = "disk is full";
        break;
    case -9:
        msg = "file system error";
        break;
    case -10:
        msg = "access denied";
        break;
    case -11:
        msg = "device error";
        break;
    case -12:
        msg = "file system requires more memory";
        break;
    case -30:
        msg = "not enough memory";
        break;
    case -31:
        msg = "file is not executable";
        break;
    case -32:
        msg = "too many processes";
        break;
    default:
        msg = "unknown error";
        break;
    }

    return msg;
}
