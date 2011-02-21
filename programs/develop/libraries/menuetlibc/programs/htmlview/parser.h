#ifndef __PARSER_H
#define __PARSER_H

class CFileStreamParser;
class CTextStreamParser;

class CParser
{
friend class CFileStreamParser;
friend class CTextStreamParser;
public:
 CParser();
 ~CParser();
 virtual char GetChar();
 void Init();
 void SkipWhite();
 int Match(char c);
 unsigned long GetNum();
 int GetToken(char * tokbuf,int maxlen);
 inline bool isEOF() { return IsEOF; }
 char Look;
private:
 bool IsEOF;
 unsigned long __GetNum();
 unsigned long __GetHexNum();
 unsigned long __GetOctNum();
};

class CFileStreamParser: public CParser
{
public:
 CFileStreamParser(FILE *);
 ~CFileStreamParser();
 virtual char GetChar();
private:
 FILE * f;
};

class CTextStreamParser: public CParser
{
public:
 CTextStreamParser(char * Stm,int stm_size);
 ~CTextStreamParser();
 virtual char GetChar();
private:
 char * __Stm;
 int __Size;
 int __Pos;
};

#endif
