#ifndef __MGTK_STRING_H
#define __MGTK_STRING_H

class GString
{
public:
 GString(char *);
 GString(const char *);
 GString(GString&);
 ~GString();
 int Length();
 char * operator = (GString&);
 GString& operator = (char *);
 void Truncate(int AtPos);
private:
 char * __str;
 int    __len;
};

#endif
