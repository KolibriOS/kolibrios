#include <stdlib.h>
#include <sys/stat.h>
#include "../../kolibri-libc/source/include/ksys.h"
#include <string.h>

void *memrchr(const void *m, int c, size_t n)
{
	const unsigned char *s = (const unsigned char*)m;
	c = (unsigned char)c;
	while (n--) if (s[n]==c) return (void *)(s+n);
	return 0;
}

void kolibri_set_win_center()
{
    ksys_proc_table_t *info = (ksys_proc_table_t*)malloc(sizeof(ksys_proc_table_t));
    _ksys_process_info(info, -1);

    ksys_pos_t screen_size= _ksys_screen_size();
    int new_x = screen_size.x/2-info->winx_size/2;
    int new_y = screen_size.y/2-info->winy_size/2;
    _ksys_change_window(new_x, new_y, -1, -1); 
    free(info);
}

int mkdir(const char * path, unsigned)
{
   return _ksys_mkdir(path);
}

char *dirname (char *path)
{
  static const char dot[] = ".";
  char *last_slash;
  /* Find last '/'.  */
  last_slash = path != NULL ? strrchr (path, '/') : NULL;
  if (last_slash != NULL && last_slash != path && last_slash[1] == '\0')
    {
      /* Determine whether all remaining characters are slashes.  */
      char *runp;
      for (runp = last_slash; runp != path; --runp)
        if (runp[-1] != '/')
          break;
      /* The '/' is the last character, we have to look further.  */
      if (runp != path)
        last_slash = (char*)memrchr((void*)path, '/', runp - path);
    }
  if (last_slash != NULL)
    {
      /* Determine whether all remaining characters are slashes.  */
      char *runp;
      for (runp = last_slash; runp != path; --runp)
        if (runp[-1] != '/')
          break;
      /* Terminate the path.  */
      if (runp == path)
        {
          /* The last slash is the first character in the string.  We have to
             return "/".  As a special case we have to return "//" if there
             are exactly two slashes at the beginning of the string.  See
             XBD 4.10 Path Name Resolution for more information.  */
          if (last_slash == path + 1)
            ++last_slash;
          else
            last_slash = path + 1;
        }
      else
        last_slash = runp;
      last_slash[0] = '\0';
    }
  else
    /* This assignment is ill-designed but the XPG specs require to
       return a string containing "." in any case no directory part is
       found and so a static and constant string is required.  */
    path = (char *) dot;
  return path;
}

void setcwd(char* path){
    _ksys_setcwd(path);
}
