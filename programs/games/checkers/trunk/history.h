#ifndef _HEADER_HISTORY_H
#define _HEADER_HISTORY_H

#ifndef __MENUET__
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#endif
#include "position.h"
#include "hash.h"
#include "sysproc.h"

class THistory
{
#ifndef __MENUET__
public:
  static char FileName[1024];
#endif
public:
  THistory(int id = 0) {if (id >= 0) Hid = NHid++; else Hid = id;}

  int GetId() const {return Hid;}
  static int GetNId() {return NHid;}

  int Start(const Position &pos) const;
  int Move(const Position &pos, const unsigned char mv[], int nmove) const;
  int Play(const PlayWrite &play) const;

#ifndef __MENUET__
  static int InitHFile(char *dname = 0);
  static int HRead(FILE *f, PlayWrite *&play);
protected:
  int Print(const char *str) const;
#endif

  int Hid;

  static int NHid;
protected:
  struct TStr
  {
    TStr(const char *ss = 0) : s(0) {(*this) = ss;}
    TStr(const TStr &ss) : s(0) {(*this) = ss.s;}
    ~TStr() {(*this) = 0;}

    TStr &operator=(const char *ss);
    TStr &operator=(const TStr &ss) {return (*this) = ss.s;}

    operator char*() {return s;}
    operator const char*() const {return s;}
    char &operator*() {return *s;}
    const char &operator*() const {return *s;}
    char &operator[](int i) {return s[i];}
    const char &operator[](int i) const {return s[i];}
    void Extend(int n);

    friend int operator==(const TStr &s1, const TStr &s2)
        {return strcmp(s1, s2) == 0;}
    friend int operator==(const char *s1, const TStr &s2)
        {return strcmp(s1, s2) == 0;}
    friend int operator==(const TStr &s1, const char *s2)
        {return strcmp(s1, s2) == 0;}
    friend int operator!=(const TStr &s1, const TStr &s2)
        {return strcmp(s1, s2) != 0;}
    friend int operator!=(const char *s1, const TStr &s2)
        {return strcmp(s1, s2) != 0;}
    friend int operator!=(const TStr &s1, const char *s2)
        {return strcmp(s1, s2) != 0;}
    friend int operator>=(const TStr &s1, const TStr &s2)
        {return strcmp(s1, s2) >= 0;}
    friend int operator>=(const char *s1, const TStr &s2)
        {return strcmp(s1, s2) >= 0;}
    friend int operator>=(const TStr &s1, const char *s2)
        {return strcmp(s1, s2) >= 0;}
    friend int operator<=(const TStr &s1, const TStr &s2)
        {return strcmp(s1, s2) <= 0;}
    friend int operator<=(const char *s1, const TStr &s2)
        {return strcmp(s1, s2) <= 0;}
    friend int operator<=(const TStr &s1, const char *s2)
        {return strcmp(s1, s2) <= 0;}
    friend int operator>(const TStr &s1, const TStr &s2)
        {return strcmp(s1, s2) > 0;}
    friend int operator>(const char *s1, const TStr &s2)
        {return strcmp(s1, s2) > 0;}
    friend int operator>(const TStr &s1, const char *s2)
        {return strcmp(s1, s2) > 0;}
    friend int operator<(const TStr &s1, const TStr &s2)
        {return strcmp(s1, s2) < 0;}
    friend int operator<(const char *s1, const TStr &s2)
        {return strcmp(s1, s2) < 0;}
    friend int operator<(const TStr &s1, const char *s2)
        {return strcmp(s1, s2) < 0;}

    char *s;
  };

  class THash
  {
  public:
    void init(int _m);
    int operator()(const TStr &str) const;
  protected:
    int m;
    int K[16];
  };

  struct TTableItem
  {
    TTableItem(const char *s = 0, int k = 0) : str(s), k(k) {}
    TTableItem(const TStr &s, int k = 0) : str(s), k(k) {}
    TTableItem(const TTableItem &t) : str(t.str), k(t.k) {}

    operator TStr&() {return str;}
    operator const TStr&() const {return str;}

    TStr str;
    int k;
  };
};

#ifndef __MENUET__
char THistory::FileName[1024] = "history.che";
#endif
int THistory::NHid = 0;

#ifndef __MENUET__
int THistory::Print(const char *str) const
{
  char *line = new char[30 + strlen(str)];
  if (!line) return 0;
  unsigned long pr_id = GetProcessId();
  if (Hid == -1) sprintf(line, "%lu  %s\n", pr_id, str);
  else if (Hid < 0) sprintf(line, "%lu%c  %s\n", pr_id, (char)Hid, str);
  else sprintf(line, "%lu:%d  %s\n", pr_id, Hid, str);
  FILE *f = fopen(FileName, "at");
  if (!f)
  {
    clock_t cc = clock();
    do {f = fopen(FileName, "at");}
    while(!f && (clock() - cc) <= 0.05 * CLOCKS_PER_SEC);
  }
  if (!f) {delete[] line; return 0;}
  fputs(line, f);
  fclose(f);
  delete[] line;
  return 1;
}
#endif

int THistory::Start(const Position &pos) const
{
  char str[20 + NUM_CELL] = "Start ";
  if (!pos.Write(str + strlen(str), 1)) return 0;
#ifndef __MENUET__
  if (!Print(str)) return 0;
#endif
  return 1;
}

int THistory::Move(const Position &pos, const unsigned char mv[], int nmove) const
{
  char *str = new char[15 + pos.GetLenMvEx(mv, 11)];
  if (!str) return 0;
  sprintf(str, "%d.%s ", (nmove + 1) / 2, (nmove % 2 == 0) ? ".." : "");
  pos.WriteMvEx(mv, str + strlen(str), 11);
#ifndef __MENUET__
  if (!Print(str)) {delete[] str; return 0;}
#endif
  delete[] str;
  return 1;
}

int THistory::Play(const PlayWrite &play) const
{
  if (play.GetN() <= 0) return 0;
  Position pos;
  if (play.GetPos(pos, 0) < 0) return 0;
  if (!Start(pos)) return 0;
  int i;
  unsigned char mv[NUM_CELL];
  for (i = 1; i < play.GetN(); i++)
  {
    if (play.GetPos(pos, i - 1) < 0) return 0;
    if (play.GetMove(mv, i) < 0) return 0;
    if (!Move(pos, mv, i)) return 0;
  }
  return 1;
}

#ifndef __MENUET__
int THistory::InitHFile(char *dname)
{
  if (dname && dname[0])
  {
    char fnm[1024];
    strcpy(fnm, dname);
    int i;
    for (i = strlen(fnm) - 1; i >= 0; i--)
    {
      if (fnm[i] == DIR_SEPARATOR) break;
    }
    if (i >= 0)
    {
      strcpy(fnm + i + 1, FileName);
      strcpy(FileName, fnm);
    }
  }
  int e = 1;
  FILE *f = fopen(FileName, "rt");
  if (f) {e = feof(f); fclose(f);}
  if (!e) return 0;
  f = fopen(FileName, "wt");
  if (!f) return -1;
  fputs("checkers-history_1.1\n", f);
  fclose(f);
  return 1;
}
#endif

THistory::TStr &THistory::TStr::operator=(const char *ss)
{
  if (s) delete[] s;
  if (ss)
  {
    s = new char[strlen(ss) + 1];
    strcpy(s, ss);
  }
  else s = 0;
  return *this;
}

void THistory::TStr::Extend(int n)
{
  if (n <= 0) {(*this) = 0; return;}
  char *ss = s;
  s = new char[n+1];
  if (ss)
  {
    strncpy(s, ss, n);
    s[n] = 0;
    delete[] ss;
  }
  else s[0] = 0;
}

void THistory::THash::init(int _m)
{
  m = _m;
  for (int i = 0; i < 16; i++)
  {
    K[i] = (2*random(32767) + 1) & ((1 << m) - 1);
  }
}

int THistory::THash::operator()(const TStr &str) const
{
  int i, r = 0;
  const char *s = str;
  for (i = 0; *s; i = (i+1) & 15) r += *(s++) * K[i];
  r &= (1 << m) - 1;
  return r;
}

#ifndef __MENUET__
int THistory::HRead(FILE *f, PlayWrite *&play)
{
  const int MAX_INP_WORD = 100;
  int nplay = 0, mplay = 10;
  play = new PlayWrite[mplay];
  THashTable<TTableItem, TStr, THash> table;
  TStr word;
  char inp_word[MAX_INP_WORD + 1];
  int r, maxword = 0;
  unsigned char ch;
  int i, k = 0, kind = 0, wasspace = 1, nmove = 0;
  for (;;)
  {
    r = (fread(&ch, 1, 1, f) == 1);
    if (!r || isspace(ch))
    {
      if (!wasspace)
      {
        if (kind == 0) kind = 1;
        else if (kind == 2)
        {
          for (i = 0; inp_word[i]; i++)
          {
            inp_word[i] = (char)tolower((unsigned char)inp_word[i]);
          }
          if (strcmp(inp_word, "start") == 0) kind = 5;
          else kind = -1;
          inp_word[0] = 0; k = 0;
        }
        else if (kind == 3)
        {
          nmove *= 2;
          if (k <= 1) nmove--;
          inp_word[0] = 0;
          k = 0; kind = 4;
        }
        else if (kind == 4)
        {
          TTableItem *n_pl = table.find(word);
          if (!n_pl) kind = -1;
          else if (nmove < 1 || nmove > play[n_pl->k].GetN()) kind = -1;
          else
          {
            PlayWrite::PMv pmv;
            if (play[n_pl->k].GetPos(pmv.pos, nmove - 1) < 0) kind = -1;
            else if (!pmv.pos.ReadMv(pmv.mv, inp_word, 1)) kind = -1;
            else
            {
              play[n_pl->k].ClearFrom(nmove);
              if (play[n_pl->k].Add(pmv.mv) != 0) kind = -1;
              else {k = n_pl->k; kind = 11;}
            }
          }
        }
        else if (kind == 5)
        {
          Position pos;
          pos.Read(inp_word, 1);
          if (pos.IsNull()) kind = -1;
          else
          {
            TTableItem *n_pl = table.find(word);
            if (!n_pl)
            {
              table.push(TTableItem(word, nplay));
              n_pl = table.find(word);
            }
            if (!n_pl) kind = -1;
            else
            {
              if (nplay >= mplay)
              {
                PlayWrite *play0 = play;
                int mplay0 = mplay;
                mplay = 2*nplay + 3;
                play = new PlayWrite[mplay];
                if (play0)
                {
                  for (i = 0; i < mplay0; i++) play[i] = play0[i];
                  delete[] play0;
                }
              }
              n_pl->k = nplay++;
              play[n_pl->k].Add(0, pos);
              k = n_pl->k; kind = 12;
            }
          }
        }
      }
      if (!r || ch == '\n' || ch == '\r')
      {
        k = 0;
        kind = 0;
        if (!r) break;
      }
      wasspace = 1;
    }
    else
    {
      if (kind == 0)
      {
        if (k >= maxword) word.Extend(2*k + 3);
        word[k++] = ch;
        word[k] = 0;
      }
      else if (kind == 1)
      {
        if (isdigit(ch)) {nmove = ch - '0'; k = 0; kind = 3;}
        else
        {
          inp_word[0] = ch;
          inp_word[1] = 0;
          k = 1; kind = 2;
        }
      }
      else if (kind == 2 || kind == 4 || kind == 5)
      {
        if (k < MAX_INP_WORD)
        {
          inp_word[k++] = ch;
          inp_word[k] = 0;
        }
      }
      else if (kind == 3)
      {
        if (k == 0 && isdigit(ch)) nmove = 10 * nmove + ch - '0';
        else if (ch == '.') k++;
        else kind = -1;
      }
      wasspace = 0;
    }
  }
  return nplay;
}
#endif

#endif  //_HEADER_HISTORY_H
