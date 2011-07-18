#include <string.h>
#include "tp.h"

extern tp_obj tp_fnc(TP,tp_obj v(TP));
extern tp_obj tp_method(TP,tp_obj self,tp_obj v(TP));
extern tp_obj tp_number(tp_num v);
extern tp_obj tp_string(char const *v);
extern tp_obj tp_list(TP);
extern tp_obj tp_dict(TP);
extern void _tp_raise(TP,tp_obj);
#define _cdecl __attribute__((cdecl))
extern int (* _cdecl con_printf)(const char* format,...);

#define call70(par, st) asm volatile ("int   $0x40":"=a"(st):"a"(70), "b"(par))
#define call70_rw(par, st, cnt) asm volatile ("int   $0x40":"=a"(st), "=b"(cnt):"a"(70), "b"(par))
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

typedef struct {
        uint32_t subfnc;
        uint32_t res1;
        uint32_t res2;
        uint32_t res3;
        uint8_t *data;
        uint8_t  res4;
        char    *fn;
    }__attribute__((__packed__))  info_params_t;

typedef struct {
        uint32_t subfnc;
        uint32_t pos;
        uint32_t res1;
        uint32_t cnt;
        char*    data;
        uint8_t res2;
        char*    fn;
    } __attribute__((__packed__)) rw_params_t;

static tp_obj kolibri_close(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    uint32_t size = tp_get(tp, self, tp_string("size")).number.val;
    char *mode = (char *)tp_get(tp, self, tp_string("mode")).string.val;
    tp_set(tp, self, tp_string("closed"), tp_True);
    return tp_None;
}

static tp_obj kolibri_read(TP)
{
    uint32_t status, num;
    tp_obj self = TP_TYPE(TP_DICT);
    uint32_t pos = tp_get(tp, self, tp_string("pos")).number.val;
    uint32_t size = tp_get(tp, self, tp_string("size")).number.val;
    uint32_t cnt;
    char *buf = (char *)malloc(size - pos);
    rw_params_t params = {0, pos, 0, size - pos, buf, 0,
                (char *)tp_get(tp, self, tp_string("name")).string.val};
    char *mode = (char *)tp_get(tp, self, tp_string("mode")).string.val;

    if (*mode != 'r')
        tp_raise(tp_None, "IOError: file not open for reading", tp_None);

    if (!buf)
        return tp_None;
    call70_rw((&params), status, cnt);
    buf[cnt] = '\0';
    return tp_string(buf);
}

#if 0
static tp_obj kolibri_readline(TP)
{
    return tp_string("Line");
}

static tp_obj kolibri_readlines(TP)
{
    tp_obj result = tp_list(tp);
    int i;

    for(i=0; i < 5; i++)
        tp_add(result, tp_string("Line"));
    return result;
}
#endif

/* Write object to file.
 *
 * tp   TinyPy virtual machine structure.
 *
 * returns tp_True
 */
static tp_obj kolibri_write(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj obj = TP_OBJ(); /* What to write. */
    char *mode = (char *)tp_get(tp, self, tp_string("mode")).string.val;
    uint32_t pos = (uint32_t)tp_get(tp, self, tp_string("pos")).number.val;
    uint32_t size = (uint32_t)tp_get(tp, self, tp_string("size")).number.val;

    if (*mode != 'w' && *mode != 'a')
    {
        tp_raise(tp_None, "IOError: file not open for writing", tp_None);
    }
    else
    {
        char *data = (char *)TP_CSTR(obj);
        rw_params_t params = {3, pos, 0, strlen(data), data, 0,
                             (char *)tp_get(tp, self, tp_string("name")).string.val};
        uint32_t status;
        uint32_t cnt;
        call70_rw((&params), status, cnt);
        if (status)
        {
            tp_raise(tp_None, "IOError: writing failed with status %d", status);
        }
        pos += cnt;
        tp_set(tp, self, tp_string("pos"), tp_number(pos));
        if (pos > size)
        {
            /* If writing has come beyond the file, increase its size. */
            tp_set(tp, self, tp_string("size"), tp_number(pos));
        }
        return tp_True;
    }
}

/* Write line list into file.
 *
 * tp   TinyPy virtual machine structure.
 *
 * returns tp_None.
 */
static tp_obj kolibri_writelines(TP)
{
    tp_obj result = tp_None;
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj list = TP_TYPE(TP_LIST); /* What to write. */
    char *mode = (char *)tp_get(tp, self, tp_string("mode")).string.val;
    long pos = (long)tp_get(tp, self, tp_string("pos")).number.val;
    uint32_t size = (uint32_t)tp_get(tp, self, tp_string("size")).number.val;

    if (*mode != 'w' && *mode != 'a')
    {
        tp_raise(tp_None, "IOError: file not open for writing", tp_None);
    }
    else
    {
        int i;
        char *fn = (char *)tp_get(tp, self, tp_string("name")).string.val;
        rw_params_t params = {3, 0, 0, 0, NULL, 0, fn};

        for (i = 0; i < list.list.val->len; i++)
        {
            char *data = (char *)TP_CSTR(list.list.val->items[i]);
            uint32_t status;
            uint32_t cnt;

            params.data = data;
            params.cnt = strlen(data);
            params.pos = pos;

            call70_rw((&params), status, cnt);
            if (status)
                tp_raise(tp_None, "IOError: writing failed with status %d", status);
            pos += cnt;
        }
        tp_set(tp, self, tp_string("pos"), tp_number(pos));
        if (pos > size)
        {
            /* If writing has come beyond the file, increase its size. */
            tp_set(tp, self, tp_string("size"), tp_number(pos));
        }
        return tp_True;
    }
}

/* Get file size.
 *
 * fn   ASCIIZ absolute file path.
 */
long long int kolibri_filesize(const char *fn)
{
    uint8_t data[40];
    uint32_t status;
    long long int result;
    info_params_t params = {5, 0, 0, 0, data, 0, (char *)fn};

    call70((&params), status);
    /* File size is at offset 32. */
    if (status == 0)
        result = *(long long*)(&data[32]);
    else
        result = -status;
    return result;
}


/* Open file.
 *
 * tp   TinyPy virtual machine structure.
 *
 * returns file object.
 */
tp_obj kolibri_open(TP) {
    tp_obj obj;
    char *fn = (char *)(TP_TYPE(TP_STRING).string.val);
    tp_obj mode_obj = TP_OBJ();
    long long int size;
    long long int pos = 0;
    uint32_t status;
    info_params_t params = {2, 0, 0, 0, NULL, 0, fn};

    if (mode_obj.type == TP_NONE)
        mode_obj = tp_string("r");
    else if (mode_obj.type != TP_STRING)
        tp_raise(tp_None, "ValueError: bad file access mode %s", TP_CSTR(mode_obj));
    switch(mode_obj.string.val[0])
    {
        case 'w':
            call70((&params), status);
            if (status)
                tp_raise(tp_None, "IOError: cannot open file for writing", tp_None);
            size = 0;
            break;
        case 'a':
            pos = size;
            break;
        case 'r':
            break;
        default:
            tp_raise(tp_None, "ValueError: mode string must begin with 'r', 'w', or 'a'", tp_None);
    }
    if ((size = kolibri_filesize(fn)) < 0)
        tp_raise(tp_None, "IOError: filesize returned %d", tp_number(size));
    obj = tp_dict(tp);
    tp_set(tp, obj, tp_string("name"), tp_string(fn));
    tp_set(tp, obj, tp_string("size"), tp_number(size));
    tp_set(tp, obj, tp_string("pos"), tp_number(pos));
    tp_set(tp, obj, tp_string("mode"), mode_obj);
#if 0
    tp_set(tp, obj, tp_string("__doc__"),
           tp_string("File object.\nAttributes:\n  name: file name\n"
                     "  closed: boolean indicating whether file was closed\n"
                     "Methods:\n  read: read the entire file into string\n"
                     "  readlines: get list of lines\n"
                     "  close: close the file\n"
                     ));
#endif
    tp_set(tp, obj, tp_string("closed"), tp_False);
    tp_set(tp, obj, tp_string("close"), tp_method(tp, obj, kolibri_close));
    tp_set(tp, obj, tp_string("read"), tp_method(tp, obj, kolibri_read));
    tp_set(tp, obj, tp_string("write"), tp_method(tp, obj, kolibri_write));
    tp_set(tp, obj, tp_string("writelines"), tp_method(tp, obj, kolibri_writelines));
    return obj;
}


