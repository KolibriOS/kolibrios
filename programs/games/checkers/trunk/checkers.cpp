#define BUILD_RUS
#ifndef __MENUET__
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#else
#include <menuet.h>
#include <me_heap.h>
using namespace Menuet;
#define strlen StrLen
#define strcpy StrCopy
#define memcpy MemCopy
#include <stdarg.h>
const unsigned dectab[] = { 1000000000, 100000000, 10000000, 1000000, 100000,
                   10000, 1000, 100, 10, 0 };
int sprintf( char *Str, char* Format, ... )
{
        int i, fmtlinesize, j, k, flag;
        unsigned head, tail;
        char c;
        va_list arglist;
        //
        va_start(arglist, Format);

        //
        fmtlinesize = strlen( Format );
        //
        if( fmtlinesize == 0 ) return 0;
  
        //
        for( i = 0, j = 0; i < fmtlinesize; i++ )
        {
                //
                c = Format[i];
                //
                if( c != '%' )
                {
                        Str[j++] = c;
                        continue;
                }
                //
                i++;
                //
                if( i >= fmtlinesize ) break;

                //
                flag = 0;
                //
                c = Format[i];
                if (c == 'l') c = Format[++i];
                //
                switch( c )
                {
                //
                case '%':
                        Str[j++] = c;
                        break;
                // ˜„ Ÿ‰•
                case 's':
                        char* str;
                        str = va_arg(arglist, char*);
                        for( k = 0; ( c = str[k] ) != 0; k++ )
                        {
                                Str[j++] = c;
                        }
                        break;
                // ˜„ Ÿ•‹Šž
                case 'c':
                        Str[j++] = va_arg(arglist, int) & 0xFF;
                        break;
                // ˜„ „ˆŒ– ŸŠž  „…Ÿš•‚Œ‹ •„…
                case 'u':
                case 'd':
                        head = va_arg(arglist, unsigned);
                        for( k = 0; dectab[k] != 0; k++ )
                        {
                                tail = head % dectab[k];
                                head /= dectab[k];
                                c = head + '0';
                                if( c == '0' )
                                {
                                        if( flag ) Str[j++] = c;
                                }
                                else
                                {
                                        flag++;
                                        Str[j++] = c;
                                }
                                //
                                head = tail;
                        }
                        //
                        c = head + '0';
                        Str[j++] = c;
                        break;
                default:
                        break;
                }
        }
        //
        Str[j] = 0;
        return j;
}
int isdigit(int c)
{
	return (c>='0' && c<='9');
}
int atoi(const char* string)
{
	int res=0;
	int sign=0;
	const char* ptr;
	for (ptr=string; *ptr && *ptr<=' ';ptr++);
	if (*ptr=='-') {sign=1;++ptr;}
	while (*ptr >= '0' && *ptr <= '9')
	{
		res = res*10 + *ptr++ - '0';
	}
	if (sign) res = -res;
	return res;
}
int islower(int c)
{
	return (c>='a' && c<='z');
}
int abs(int n)
{
	return (n<0)?-n:n;
}
int memcmp(const void* buf1, const void* buf2, unsigned count)
{
	const char* ptr1 = (const char*)buf1;
	const char* ptr2 = (const char*)buf2;
	unsigned i=0;
	while (i<count && *ptr1==*ptr2) {++i;++ptr1;++ptr2;}
	if (i==count)
		return 0;
	else if (*ptr1<*ptr2)
		return -1;
	else
		return 1;
}
void strncpy(char* dest, const char* source, unsigned len)
{
	char* ptr1 = dest;
	const char *ptr2 = source;
	for (;len-- && *ptr2; *ptr1++=*ptr2++) ;
}
unsigned int rand_data[4];

void randomize()
{
	rand_data[0] = (unsigned int)Clock();
	rand_data[1] = (unsigned int)GetPackedTime();
	rand_data[2] = (unsigned int)GetPackedDate();
	rand_data[3] = (unsigned int)0xA3901BD2 ^ GetPid();
}

unsigned int rand()
{
	rand_data[0] ^= _HashDword(rand_data[3] + 0x2835C013U);
	rand_data[1] += _HashDword(rand_data[0]);
	rand_data[2] -= _HashDword(rand_data[1]);
	rand_data[3] ^= _HashDword(rand_data[2]);
	return rand_data[3];
}

#define random(k)  (rand() % (k))
#define floor Floor
double fabs(double x)
{
	__asm	fld	x
	__asm	fabs
}
#define M_PI       3.14159265358979323846
double cos(double x)
{
	__asm	fld	x
	__asm	fcos
}
double sin(double x)
{
	__asm	fld	x
	__asm	fsin
}
/*inline void printf(const char* format, ...)
{}*/
#define printf /* nothing */
inline void strcat(char* str1, const char* str2)
{strcpy(str1+strlen(str1),str2);}
int strncmp(const char* str1, const char* str2, unsigned len)
{
	for (;len--;)
	{
		if (*str1 != *str2) break;
		if (*str1 == 0)
			return 0;
		++str1;++str2;
	}
	if (len==(unsigned)-1)
		return 0;
	if (*str1 < *str2)
		return -1;
	return 1;
}
#define clock Clock
typedef unsigned int clock_t;
#define CLOCKS_PER_SEC 100
#define XK_Left		0xB0
#define XK_Right	0xB3
#define XK_Up		0xB2
#define XK_Down		0xB1
#define XK_Return	0x0D
#define XK_space	0x20
#define XK_Escape	0x1B
#define XK_less		'<'
#define XK_comma	','
#define XK_period	'.'
#define XK_greater	'>'
#define XK_minus	'-'
#define XK_equal	'='
#define XK_underscore	'_'
#define XK_plus		'+'
#define XK_Delete	0xB6
#define XK_F8		0x39
#define XK_l		'l'
#define XK_L		'L'
#define XK_F2		0x33
#define XK_s		's'
#define XK_S		'S'
#define XK_slash	'/'
#define XK_question	'?'
#define XK_n		'n'
#define XK_N		'N'
#define XK_t		't'
#define XK_T		'T'
#define XK_r		'r'
#define XK_R		'R'
#define XK_b		'b'
#define XK_B		'B'
#define XK_f		'f'
#define XK_F		'F'
#define assert(a) /* nothing */
#include "qsort.c"
#endif
#include "gr-draw.h"
#include "board.h"
#include "player.h"
#include "buttons.h"
#include "sysproc.h"

char *strskipref(char *s1, char *s2)
{
  int L = strlen(s2);
  if (strncmp(s1, s2, L) == 0) return s1 + L;
  else return 0;
}

class TPlayArray
{
public:
  TPlayArray(int d = 10) : play(0), nplay(0), mplay(0) {}
  ~TPlayArray() {Clear();}

  void Clear();
  void Add(const PlayWrite &pl);
  PlayWrite &operator[](int i) {return play[i];}
  int GetNPlay() const {return nplay;}
#ifndef __MENUET__
  int OpenFile(const char *name, int kind);
  int MsgOpenFile(const char *name, int kind);
  int SaveFile(const char *name, int num, int kind);
  int MsgSaveFile(const char *name, int num, int kind);
#endif
  void Del(int n);
protected:
  void Extend(int n = -1);

  PlayWrite *play;
  int nplay, mplay;
protected:
#ifndef __MENUET__
  static const char *const search[];
  static int AnalizeKind(FILE *f, const int w[], const char *smb = 0);
  static int ReadFileTo(FILE *f, char c);
  static int ReadFileTo(FILE *f, const char *c);
  static int ReadTheText(FILE *f, const char *s);
#endif
};

#ifndef __MENUET__
const char *const TPlayArray::search[] =
     {"checkers-", "play:", "text", "bin_1.0\n", "history_", "1.0", "1.1"};
#endif

void TPlayArray::Clear()
{
  if (play) delete[] play;
  play = 0; nplay = 0; mplay = 0;
}

void TPlayArray::Extend(int n)
{
  if (n < 0) n = nplay + 1;
  if (mplay < n)
  {
    PlayWrite *p_old = play;
    int m_old = mplay;
    mplay = n * 2;
    play = new PlayWrite[mplay];
    if (p_old)
    {
      for (int i = 0; i < m_old; i++) play[i] = p_old[i];
      delete[] p_old;
    }
  }
}

void TPlayArray::Add(const PlayWrite &pl)
{
  Extend();
  play[nplay] = pl;
  nplay++;
}

#ifndef __MENUET__
int TPlayArray::AnalizeKind(FILE *f, const int w[], const char *smb)
{
  int i, L, was1 = 1;
  unsigned char ch;
  int fnd[NELEM(search)];
  for (i = 0; i < NELEM(search); i++) fnd[i] = 1;
  for (L = 0; was1; L++)
  {
    was1 = 0;
    if (fread(&ch, 1, 1, f) != 1) return (L == 0) ? -2 : -1;
    for (i = 0; w[i] >= 0; i++) if (fnd[i])
    {
      if (tolower(ch) != tolower(search[w[i]][L])) fnd[i] = 0;
      else
      {
        was1 = 1;
        if (search[w[i]][L+1] == 0)
        {
          if (smb && smb[0] && fread(&ch, 1, 1, f) == 1)
          {
            ungetc(ch, f);
            if (strchr(smb, tolower(ch)) || isalpha(ch) && strchr(smb, toupper(ch)))
            {
              break;
            }
          }
          return i;
        }
      }
    }
  }
  return -10 - L;
}

int TPlayArray::ReadFileTo(FILE *f, char c)
{
  char ch;
  do
  {
    if (fread(&ch, 1, 1, f) != 1) return 0;
  }
  while(ch != c);
  return 1;
}

int TPlayArray::ReadFileTo(FILE *f, const char *c)
{
  char ch;
  do
  {
    if (fread(&ch, 1, 1, f) != 1) return 0;
  }
  while(!strchr(c, ch));
  return 1;
}

int TPlayArray::ReadTheText(FILE *f, const char *s)
{
  char ch;
  while (*s)
  {
    if (fread(&ch, 1, 1, f) != 1 || ch != *s) return 0;
    s++;
  }
  return 1;
}

int TPlayArray::OpenFile(const char *name, int kind)
{
  int k_fnd = 0;
  FILE *f = fopen(name, "rb");
  if (!f) return -1;
  if (kind == -1)
  {
    const int w[] = {0, -1};
    int r = AnalizeKind(f, w);
    if (r >= 0) {kind = w[r]; k_fnd = 1;}
    else if (r == -11) {kind = 3; k_fnd = 3;}
    else {fclose(f); return (kind == -2) ? -2 : -10;}
  }
  if (kind == 0)
  {
    if (k_fnd < 1 && !ReadFileTo(f, '-')) {fclose(f); return -10;}
    k_fnd = 1;
    const int w[] = {1, 4, -1};
    int r = AnalizeKind(f, w);
    if (r >= 0) {kind = w[r]; k_fnd = 2;}
    else {fclose(f); return -10;}
  }
  if (kind == 1)
  {
    if (k_fnd < 2 && !ReadFileTo(f, ':')) {fclose(f); return -10;}
    k_fnd = 2;
    const int w[] = {2, 3, -1};
    int r = AnalizeKind(f, w);
    if (r >= 0) {kind = w[r]; k_fnd = 3;}
    else {fclose(f); return -10;}
  }
  if (kind == 4)
  {
    if (k_fnd < 2 && !ReadFileTo(f, '_')) {fclose(f); return -10;}
    k_fnd = 2;
    const int w[] = {5, 6, -1};
    int r = AnalizeKind(f, w, ".1234567890");
    if (r >= 0) {kind = w[r]; k_fnd = 3;}
    else {fclose(f); return -10;}
  }
  if (kind == 5) {kind = 3; k_fnd = 0;}
  if (kind == 6)
  {
    if (!ReadFileTo(f, "\n\r")) {fclose(f); return -4;}
    k_fnd = 3;
    PlayWrite *pl = 0;
    int np = THistory::HRead(f, pl);
    if (np > 0 && pl)
    {
      int i;
      Extend(nplay + np);
      for (i = 0; i < np; i++)
      {
        if (pl[i].GetN() >= 3) play[nplay++] = pl[i];
      }
    }
    if (pl) delete[] pl;
    fclose(f);
    return 1;
  }
  if (kind == 2)
  {
    printf("kind = 2\n");
    unsigned char ch;
    do
    {
      if (fread(&ch, 1, 1, f) != 1) {fclose(f); return -10;}
    }
    while(!isspace(ch));
    PlayWrite pl;
    char word[101];
    int i, kind = 0;
    for (;;)
    {
      do
      {
        if (fread(&ch, 1, 1, f) != 1) break;
      }
      while(ch == 0 || isspace(ch));
      if (feof(f)) strcpy(word, "[end]");
      else
      {
        i = 0;
        while(ch != 0 && !isspace(ch))
        {
          if (i < 100) word[i++] = ch;
          if (fread(&ch, 1, 1, f) != 1) break;
        }
        word[i] = 0;
      }
      if (word[0] != '[')
      {
        if (kind == 1)
        {
          if (word[0] != '#' && word[0] != '$' && word[0] != '%')
          {
            Position pos;
            pos.Read(word, 1);
            pl.Clear();
            pl.Add(0, pos);
            kind = 2;
          }
        }
        else if (kind == 2)
        {
          if (word[0] != '#' && word[0] != '$' && word[0] != '%')
          {
            for (i = 0; word[i] && word[i] != '.' && word[i] != ','; i++)
            {
              if (!isdigit((unsigned char)word[i])) {i = -1; break;}
            }
            if (i == -1)
            {
              PlayWrite::PMv pmv;
              if (pl.GetPosL(pmv.pos) < 0) kind = 3;
              else if (!pmv.pos.ReadMv(pmv.mv, word, 1)) kind = 3;
              else if (pl.Add(pmv.mv) != 0) kind = 3;
            }
          }
        }
      }
      else
      {
        if (kind == 2 || kind == 3)
        {
          if (pl.GetN() > 0) Add(pl);
          pl.Clear();
        }
        kind = 0;
        for (i = 0; word[i]; i++)
        {
          word[i] = (char)tolower((unsigned char)word[i]);
        }
        if (strskipref(word, "[end]")) break;
        else if (strskipref(word, "[play_v1.1]")) kind = 1;
      }
    }
    fclose(f);
    return 1;
  }
  if (kind == 3)
  {
    char ch[LEN_WPOS];
    if (k_fnd < 3 && !ReadFileTo(f, '\n')) {fclose(f); return -10;}
    k_fnd = 3;
    do
    {
      PlayWrite pl;
      for (;;)
      {
        int i;
        for (i = 0; i < LEN_WPOS; i++)
        {
          if (fread(ch + i, 1, 1, f) != 1 || ch[i] < 0) break;
        }
        if (i < LEN_WPOS) break;
        PlayWrite::PMv pmv0, pmv1;
        pmv1.pos.Read(ch);
        pmv1.pos.Reverse();
        if (pl.GetPMvL(pmv0) >= 0)
        {
          TComputerPlayer::Z z;
          z.FindAllMoves(pmv0);
          int r;
          for (r = 0; r < z.narr; r++)
          {
            if (memcmp(z.array[r].pos.SH, pmv1.pos.SH, sizeof(pmv1.pos.SH)) == 0)
            {
              pmv1 = z.array[r];
              break;
            }
          }
          if (r < z.narr) pl.Add(pmv1);
          else {fclose(f); return -3;}
        }
        else pl.Add(0, pmv1.pos);
      }
      if (pl.GetN() > 0) Add(pl);
    }
    while(!feof(f));
    fclose(f);
    return 1;
  }
  fclose(f);
  return -10;
}

int TPlayArray::MsgOpenFile(const char *name, int kind)
{
  int n0 = nplay, no_games = 0;
  int r = OpenFile(name, kind);
  if (r <= 0)
  {
    if (r == -1)
    {
#ifdef BUILD_RUS
      printf("\n˜ èª¨: ¥ ¬®£ã ®âªàëâì ä ©« \"%s\".\n", name);
#else
      printf("\nCheckers: Can't open the file \"%s\".\n", name);
#endif
      return 0;
    }
    else if (r == -2)
    {
#ifdef BUILD_RUS
      printf("\n˜ èª¨: ” ©« \"%s\" ¯ãáâ.\n", name);
#else
      printf("\nCheckers: The file \"%s\" is empty.\n", name);
#endif
      return 0;
    }
    else if (r == -3)
    {
#ifdef BUILD_RUS
      printf("\n˜ èª¨: ” ©« \"%s\" ®è¨¡®ç­ë©.\n", name);
#else
      printf("\nCheckers: Invalid file \"%s\".\n", name);
#endif
      return 0;
    }
    else if (r == -4) no_games = 1;
    else if (r == -10)
    {
#ifdef BUILD_RUS
      printf("\n˜ èª¨: ” ©« \"%s\" ¨¬¥¥â ­¥¢¥à­ë© â¨¯.\n", name);
#else
      printf("\nCheckers: The file \"%s\" has wrong type.\n", name);
#endif
      return 0;
    }
    else
    {
#ifdef BUILD_RUS
      printf("\n˜ èª¨: Žè¨¡ª  ®âªàëâ¨ï ä ©«  \"%s\".\n", name);
#else
      printf("\nCheckers: Error openning the file \"%s\".\n", name);
#endif
      return 0;
    }
  }
  if (!no_games && nplay > n0) return 1;
  else
  {
#ifdef BUILD_RUS
    printf("\n˜ èª¨: ” ©« \"%s\" ­¥ á®¤¥à¦¨â ¨£à.\n", name);
#else
    printf("\nCheckers: File \"%s\" don't contains games.\n", name);
#endif
    return 0;
  }
}

int TPlayArray::SaveFile(const char *name, int num, int kind)
{
  FILE *f = 0;
  if (kind == 0 || kind == 1 || kind == 2)
  {
    f = fopen(name, "wt");
    if (!f) return -1;
    fprintf(f, "checkers-play:text\n");
    int i0 = num, i;
    if (num < 0) {i0 = 0; num = nplay-1;}
    for (i = i0; i <= num; i++) if (play[i].GetN() > 0)
    {
      PlayWrite::PMv pmv;
      if (play[i].GetPos(pmv.pos, 0) < 0) return -9;
      char *str = new char[10 + NUM_CELL];
      if (!str) return -5;
      pmv.pos.Write(str, 1);
      fprintf(f, "\n[Play_v1.1]#%d  %s\n", i - i0 + 1, str);
      delete[] str;
      int j;
      for (j = 1; j < play[i].GetN(); j++)
      {
        if (play[i].GetPos(pmv.pos, j - 1) < 0) return -9;
        if (play[i].GetMove(pmv.mv, j) < 0) return -9;
        str = new char[pmv.pos.GetLenMvEx(pmv.mv, 11)];
        if (!str) return -5;
        pmv.pos.WriteMvEx(pmv.mv, str, 11);
        if (j % 2 == 1)
        {
          int nbytes = fprintf(f, "%d. ", (j + 1) / 2);
          while(nbytes++ < 5) fprintf(f, " ");
        }
        fprintf(f, "%s", str);
        if (j % 2 == 0 || j == play[i].GetN() - 1) fprintf(f, "\n");
        else fprintf(f, " ,\t");
        delete[] str;
      }
    }
    fclose(f);
    return 1;
  }
  else if (kind == -1 || kind == 3)
  {
    f = fopen(name, "wb");
    if (!f) return -1;
    if (kind == 3) fprintf(f, "checkers-play:bin_1.0\n");
    int i = num;
    if (num < 0) {i = 0; num = nplay-1;}
    for (; i <= num; i++)
    {
      char ch[LEN_WPOS];
      Position pos;
      play[i].GetPosL(pos);
      if (!pos.AllCanEat() && !pos.AllCanMove())
      {
        ch[0] = (pos.wmove == 0) ? char(-3) : char(-1);
      }
      else ch[0] = char(-2);
      fwrite(ch, 1, 1, f);
      int j;
      for (j = 0; j < play[i].GetN(); j++)
      {
        Position pos;
        play[i].GetPos(pos, j);
        pos.Reverse();
        pos.Write(ch);
        fwrite(ch, LEN_WPOS, 1, f);
      }
    }
    fclose(f);
    return 1;
  }
  if (f) fclose(f);
  return -10;
}

int TPlayArray::MsgSaveFile(const char *name, int num, int kind)
{
  int r = SaveFile(name, num, kind);
  if (r <= 0)
  {
    if (r == -1)
    {
#ifdef BUILD_RUS
      printf("\n˜ èª¨: ¥ ¬®£ã ®âªàëâì ä ©« \"%s\" ­  § ¯¨áì.\n", name);
#else
      printf("\nCheckers: Can't open the file \"%s\" to write.\n", name);
#endif
    }
    else if (r == -5)
    {
#ifdef BUILD_RUS
      printf("\n˜ èª¨: ¥ ¤®áâ â®ç­® ¯ ¬ïâ¨ ¤«ï á®åà ­¥­¨ï ä ©«  \"%s\".\n", name);
#else
      printf("\nCheckers: Not enough memory to save the file \"%s\".\n", name);
#endif
    }
    else if (r == -10)
    {
#ifdef BUILD_RUS
      printf("\n˜ èª¨: ” ©« \"%s\" ¨¬¥¥â ­¥¢¥à­ë© â¨¯.\n", name);
#else
      printf("\nCheckers: The file \"%s\" has wrong type.\n", name);
#endif
    }
    else
    {
#ifdef BUILD_RUS
      printf("\n˜ èª¨: Žè¨¡ª  á®åà ­¥­¨ï ä ©«  \"%s\".\n", name);
#else
      printf("\nCheckers: Error saving the file \"%s\".\n", name);
#endif
    }
    return 0;
  }
  else
  {
#ifdef BUILD_RUS
    printf("\n˜ èª¨: ” ©« \"%s\" á®åà ­ñ­.\n", name);
#else
    printf("\nCheckers: File \"%s\" saved.\n", name);
#endif
    return 1;
  }
}
#endif

void TPlayArray::Del(int n)
{
  if (!play || n < 0 || n >= nplay) return;
  else if (nplay <= 1) {Clear(); return;}
  int i;
  for (i = n; i < nplay - 1; i++) play[i] = play[i+1];
  play[nplay - 1].Clear();
  nplay--;
}


void SetPlayerString(char *str, int c, TChPlayer *p)
{
  strcpy(str, c ? "blue: " : "red: ");
  if (!p) strcat(str, "input");
  else strcat(str, "computer");
}

class TMainDraw : public TSomeDraw
{
public:
  TMainDraw() : undo_redo(0), ur_type(-1), cur_play(0),
                def_savefile("save.che"), def_savekind(2) {InitButton();}

  virtual void Draw(TGraphDraw *drw, int w, int h)
  {
    int d = button.GetDelt();
    int hh = button.GetHeight(w - 2*d);
    if (hh != 0) hh += d;
    drw->SetColor(drw->GetBlackColor());
    button.Draw(drw, d, h - hh, w - 2*d);
  }

  virtual void DrawB(TGraphDraw *drw, TChBoard &board)
  {
    int urt = (board.GetCurMoveN() <= 0) +
            2*(board.GetCurMoveN() >= board.GetNumMove());
    if (ur_type != urt) SetButtonKind(board);
    Draw(drw, board.GetW(), board.GetH());
  }

  virtual TIntPoint GetDSize(int w, int h)
  {
    int d = button.GetDelt();
    int hh = button.GetHeight(w - 2*d);
    if (hh != 0) hh += d;
    return TIntPoint(0, hh);
  }

  virtual int ButtonPnt(int xp, int yp, int w, int h, int &n, int k = 0)
  {
    int d = button.GetDelt();
    int hh = button.GetHeight(w - 2*d);
    if (hh != 0) hh += d;
    return button.ButtonPnt(xp - d, yp - (h - hh), w - 2*d, n, k);
  }

  void InitButton();
  void SetButtonKind(const TChBoard &board);
  void PressUR(int n, TChBoard &board, TGraphDraw *drw);
  void PressLS(int n, TChBoard &board, TGraphDraw *drw);
  void CurPlayNorm()
  {
    if (cur_play < 0 || cur_play >= play.GetNPlay()) cur_play = 0;
  }

  TXButtonArray button;
  TMultiButton *undo_redo, *play_list;
  char player_str[2][50], play_num[2][10];
  TPlayArray play;
  int cur_play;
  char *def_savefile;
  int def_savekind;
protected:
  int ur_type;
};

void TMainDraw::InitButton()
{
  button.Add(1, 80, 22, "new game");
  button.Add(6, 60, 22, "list");
  button.Add(7, 60, 22, "delete");
  play_list = new TMultiButton(20);
  play_list->a.Add(26, 20, 22, "1");
  play_list->a.Add(21, 20, 22, "-");
  play_list->a.Add(23, 37, 22, play_num[0], -2);
  play_list->a.Add(22, 20, 22, "+");
  play_list->a.Add(27, 37, 22, play_num[1]);
  play_list->SetDefW();
  button.Add(play_list);
  button.Add(24, 50, 22, "clear");
#ifndef __MENUET__
  button.Add(25, 50, 22, "save");
#endif
  button.Add(2, 120, 22, player_str[0]);
  button.Add(3, 120, 22, player_str[1]);
  button.Add(4, 110, 22, "rotate board");
  undo_redo = new TMultiButton(10);
  undo_redo->a.Add(11, 27, 22, "<<");
  undo_redo->a.Add(12, 20, 22, "<");
  undo_redo->a.Add(15, 20, 22, "^");
  undo_redo->a.Add(13, 20, 22, ">");
  undo_redo->a.Add(14, 27, 22, ">>");
  undo_redo->SetDefW();
  button.Add(undo_redo);
  button.Add(5, 60, 22, "exit");
}

void TMainDraw::SetButtonKind(const TChBoard &board)
{
  int thick;
  TextButton *txb;
  TXButton *bt1;
  ur_type = 0;
  SetPlayerString(player_str[0], 0, board.GetPlayer(0));
  SetPlayerString(player_str[1], 1, board.GetPlayer(1));
  int is_drw = !board.GetPViewStatus();
  bt1 = button.GetButton(2);
  if (bt1) bt1->drw = is_drw;
  bt1 = button.GetButton(3);
  if (bt1) bt1->drw = is_drw;
  if (board.GetCurMoveN() <= 0) {ur_type++; thick = 0;}
  else thick =  3;
//  txb = dynamic_cast<TextButton*>(undo_redo->a.GetButton(11));
//  if (txb) txb->thick = thick;
//  txb = dynamic_cast<TextButton*>(undo_redo->a.GetButton(12));
//  if (txb) txb->thick = thick;
// we can use simple static cast
	((TextButton*)(undo_redo->a.GetButton(11)))->thick = thick;
	((TextButton*)(undo_redo->a.GetButton(12)))->thick = thick;
  if (board.GetCurMoveN() >= board.GetNumMove()) {ur_type += 2; thick = 0;}
  else thick = 3;
//  txb = dynamic_cast<TextButton*>(undo_redo->a.GetButton(13));
//  if (txb) txb->thick = thick;
//  txb = dynamic_cast<TextButton*>(undo_redo->a.GetButton(14));
//  if (txb) txb->thick = thick;
	((TextButton*)(undo_redo->a.GetButton(13)))->thick = thick;
	((TextButton*)(undo_redo->a.GetButton(14)))->thick = thick;
  if (board.GetCurMoveN() < board.GetNumMove() ||
     (board.GetPViewStatus() && board.GetGameEnd() <= 0))
  {
    thick = 3;
  }
  else thick = 0;
//  txb = dynamic_cast<TextButton*>(undo_redo->a.GetButton(15));
//  if (txb) txb->thick = thick;
	((TextButton*)(undo_redo->a.GetButton(15)))->thick = thick;
  if (play.GetNPlay() == 0) is_drw = 1;
  bt1 = button.GetButton(6);
  if (bt1) bt1->drw = is_drw;
  bt1 = button.GetButton(7);
  if (bt1) bt1->drw = !is_drw;
#ifndef __MENUET__
  bt1 = button.GetButton(25);
  if (bt1) bt1->drw = !is_drw;
#endif
  is_drw = board.GetPViewStatus() && play.GetNPlay() > 1;
  bt1 = button.GetButton(20);
  if (bt1) bt1->drw = is_drw;
  bt1 = button.GetButton(24);
  if (bt1) bt1->drw = is_drw;
  if (is_drw)
  {
    play_num[0][0] = 0; play_num[1][0] = 0;
    if (cur_play >= 0) sprintf(play_num[0], "%d", cur_play + 1);
    sprintf(play_num[1], "%d", play.GetNPlay());
    thick = (cur_play <= 0) ? 0 : 3;
#define dynamic_cast static_cast
    txb = dynamic_cast<TextButton*>(play_list->a.GetButton(21));
    if (txb) txb->thick = thick;
    txb = dynamic_cast<TextButton*>(play_list->a.GetButton(26));
    if (txb) txb->thick = thick;
    thick = (cur_play >= play.GetNPlay() - 1) ? 0 : 3;
    txb = dynamic_cast<TextButton*>(play_list->a.GetButton(22));
    if (txb) txb->thick = thick;
    txb = dynamic_cast<TextButton*>(play_list->a.GetButton(27));
    if (txb) txb->thick = thick;
#undef dynamic_cast
  }
}

void TMainDraw::PressUR(int n, TChBoard &board, TGraphDraw *drw)
{
  int mv;
  if (n == 11) mv = 0;
  else if (n == 12) mv = board.GetCurMoveN() - 1;
  else if (n == 13) mv = board.GetCurMoveN() + 1;
  else mv = board.GetNumMove();
  if (board.SetCurMoveN(mv))
  {
    SetButtonKind(board);
    if (drw && drw->IsDraw())
    {
      drw->DrawClear();
      board.Draw(drw);
    }
  }
  else Draw(drw, board.GetW(), board.GetH());
}

void TMainDraw::PressLS(int n, TChBoard &board, TGraphDraw *drw)
{
  int need_redraw = 0;
  if (n == 6)
  {
    if (!board.GetPViewStatus() || play.GetNPlay() == 0)
    {
      PlayWrite cur_pw = board.GetPlay();
      if (cur_pw.GetN() > 2 || play.GetNPlay() == 0) play.Add(board.GetPlay());
      cur_play = play.GetNPlay() - 1;
      board.SetPlay(play[cur_play]);
      need_redraw = 1;
    }
  }
  else if (n == 7)
  {
    if (board.GetPViewStatus() && play.GetNPlay() != 0)
    {
      play.Del(cur_play);
      if (play.GetNPlay() >= 1)
      {
        if (cur_play >= play.GetNPlay()) cur_play--;
        board.SetPlay(play[cur_play]);
      }
      need_redraw = 1;
    }
  }
  else if (n == 21)
  {
    if (cur_play > 0) {board.SetPlay(play[--cur_play]); need_redraw = 1;}
  }
  else if (n == 22)
  {
    if (cur_play < play.GetNPlay() - 1)
    {
      board.SetPlay(play[++cur_play]); need_redraw = 1;
    }
  }
  else if (n == 26)
  {
    if (cur_play > 0)
    {
      cur_play = 0;
      board.SetPlay(play[cur_play]); need_redraw = 1;
    }
  }
  else if (n == 27)
  {
    if (cur_play < play.GetNPlay() - 1)
    {
      cur_play = play.GetNPlay() - 1;
      board.SetPlay(play[cur_play]); need_redraw = 1;
    }
  }
  else if (n == 24) {play.Clear(); cur_play = 0; need_redraw = 1;}
#ifndef __MENUET__
  else if (n == 25)
  {
    if (play.GetNPlay() > 0) play.MsgSaveFile(def_savefile, -1, def_savekind);
  }
  else if (n == 28)
  {
    if (play.GetNPlay() > 0) play.MsgSaveFile(def_savefile, cur_play, def_savekind);
  }
#endif
  if (need_redraw)
  {
    SetButtonKind(board);
    if (drw && drw->IsDraw())
    {
      drw->DrawClear();
      board.Draw(drw);
    }
  }
  else Draw(drw, board.GetW(), board.GetH());
}

struct TTimerDraw
{
  TTimerDraw(TChBoard *brd, TGraphDraw *drw) : brd(brd), drw(drw) {}

  TChBoard *brd;
  clock_t st, ut, dt;
  double x0;
  TGraphDraw *drw;

  static void draw(void *v, int k = 0);
};

void TTimerDraw::draw(void *v, int k)
{
  TTimerDraw &d = *(TTimerDraw*)v;
  clock_t t = clock();
  if (k == 0 && t - d.ut < CLOCKS_PER_SEC * 0.01) return;
  if (k > 0)
  {
    d.st = t;
    if (!d.drw || !d.drw->IsDraw()) return;
    d.drw->DrawClear();
    d.brd->Draw(d.drw);
    d.dt = t;
  }
  else if (!d.drw || !d.drw->IsDraw()) return;
  double xold = d.x0;
  if (k >= 0)
  {
    d.x0 = (1 - cos(2.0 * (t - d.st) / CLOCKS_PER_SEC)) / 2;
    d.brd->DrawTimer(d.drw, d.x0, 0);
    d.ut = t;
    if (k == 0 && t - d.dt > CLOCKS_PER_SEC * 0.5)
    {
      d.brd->Draw(d.drw);
      d.dt = t;
    }
  }
  if (k <= 0) d.brd->DrawTimer(d.drw, xold, 1);
}


struct TMainData
{
  TChBoard board;
  TComputerPlayer player;
  TMainDraw main_draw;

  TMainData(int id = 0);
  void InitDef();
  static int EventFunction(const TGraphDraw::event &ev);
  void NewGame(TGraphDraw *drw);
  void RotateBoard(TGraphDraw *drw);
  void PlayerPress(int np, TGraphDraw *drw);
  void GoToCurMove(TGraphDraw *drw);
};

TMainData::TMainData(int id) : board(id)
{
  board.SetCheckResize(1);
  board.SetPlayer(1, &player);
  board.SetBottomColor(0);
  board.SetSomeDraw(&main_draw);
  board.SetMinWSize(90, 140);
}

void TMainData::InitDef()
{
  if (main_draw.play.GetNPlay() > 0)
  {
    main_draw.CurPlayNorm();
    board.SetPlay(main_draw.play[main_draw.cur_play]);
  }
  main_draw.SetButtonKind(board);
}

void TMainData::NewGame(TGraphDraw *drw)
{
  board.NewGame();
  main_draw.SetButtonKind(board);
  if (drw && drw->IsDraw()) {drw->DrawClear(); board.Draw(drw);}
}

void TMainData::RotateBoard(TGraphDraw *drw)
{
  board.SetBottomColor(3 - board.GetBottomColor());
  if (drw && drw->IsDraw()) {drw->DrawClear(); board.Draw(drw);}
}

void TMainData::PlayerPress(int np, TGraphDraw *drw)
{
  if (np != 0 && np != 1) return;
  if (board.GetPlayer(np)) board.SetPlayer(np, 0);
  else board.SetPlayer(np, &player);
  if (board.GetPlayer(0) && !board.GetPlayer(1))
  {
    board.SetBottomColor(1);
  }
  if (board.GetPlayer(1) && !board.GetPlayer(0))
  {
    board.SetBottomColor(0);
  }
  main_draw.SetButtonKind(board);
  if (drw && drw->IsDraw()) {drw->DrawClear(); board.Draw(drw);}
}

void TMainData::GoToCurMove(TGraphDraw *drw)
{
  board.GoToCurMove();
  main_draw.SetButtonKind(board);
  if (drw && drw->IsDraw()) {drw->DrawClear(); board.Draw(drw);}
}

int TMainData::EventFunction(const TGraphDraw::event &ev)
{
  if (!ev.any.drw->data) return -100;
  TMainData &data = *(TMainData*)ev.any.drw->data;
  int nbutton, ret = 0;
  switch(ev.type)
  {
  case TGraphDraw::event::button_down:
    if (ev.button.n != 1) break;
    ev.button.drw->OpenDraw();
    if (data.main_draw.ButtonPnt(ev.button.x, ev.button.y,
             data.board.GetW(), data.board.GetH(), nbutton, 1) > 0)
    {
      data.main_draw.Draw(ev.button.drw, data.board.GetW(), data.board.GetH());
      ret |= TGraphDraw::ret_setcapture;
    }
    else data.board.MouseClick(ev.button.drw, ev.button.x, ev.button.y);
    ev.button.drw->CloseDraw();
    break;
  case TGraphDraw::event::mouse_move:
    if (ev.button.n >= 0 && ev.button.n != 1) break;
    ev.button.drw->OpenDraw();
    if (data.main_draw.ButtonPnt(ev.button.x, ev.button.y,
             data.board.GetW(), data.board.GetH(), nbutton, 2) >= 1000)
    {
      data.main_draw.Draw(ev.button.drw, data.board.GetW(), data.board.GetH());
    }
    ev.button.drw->CloseDraw();
    break;
  case TGraphDraw::event::button_up:
    if (ev.button.n != 1) break;
    ev.button.drw->OpenDraw();
    if (data.main_draw.ButtonPnt(ev.button.x, ev.button.y,
             data.board.GetW(), data.board.GetH(), nbutton, 3) > 0)
    {
      switch(nbutton)
      {
      case 1:
        data.NewGame(ev.button.drw);
        break;
      case 2:
      case 3:
        data.PlayerPress(nbutton - 2, ev.button.drw);
        break;
      case 4:
        data.RotateBoard(ev.button.drw);
        break;
      case 5:
        data.main_draw.Draw(ev.button.drw, data.board.GetW(), data.board.GetH());
        ev.button.drw->Quit();
        break;
      case 11:
      case 12:
      case 13:
      case 14:
        data.main_draw.PressUR(nbutton, data.board, ev.button.drw);
        break;
      case 15:
        data.GoToCurMove(ev.button.drw);
        break;
      case 6:
      case 7:
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
        data.main_draw.PressLS(nbutton, data.board, ev.button.drw);
        break;
      default:
        data.main_draw.Draw(ev.button.drw, data.board.GetW(), data.board.GetH());
        break;
      }
    }
    ev.button.drw->CloseDraw();
    break;
  case TGraphDraw::event::draw:
    ev.button.drw->OpenDraw();
    data.board.Draw(ev.button.drw);
    ev.button.drw->CloseDraw();
    break;
  case TGraphDraw::event::key_down:
    ev.button.drw->OpenDraw();
    if (ev.key.k == XK_Left) data.board.PKeyEvent(ev.button.drw, TChBoard::PLeft);
    else if (ev.key.k == XK_Right) data.board.PKeyEvent(ev.button.drw, TChBoard::PRight);
    else if (ev.key.k == XK_Up) data.board.PKeyEvent(ev.button.drw, TChBoard::PUp);
    else if (ev.key.k == XK_Down) data.board.PKeyEvent(ev.button.drw, TChBoard::PDown);
    else if (ev.key.k == XK_Return || ev.key.k == XK_space)
    {
      data.board.PKeyEvent(ev.button.drw, TChBoard::PEnter);
    }
    else if (ev.key.k == XK_Escape) ev.button.drw->Quit();
    else if (ev.key.k == XK_less) data.main_draw.PressUR(11, data.board, ev.button.drw);
    else if (ev.key.k == XK_comma) data.main_draw.PressUR(12, data.board, ev.button.drw);
    else if (ev.key.k == XK_period) data.main_draw.PressUR(13, data.board, ev.button.drw);
    else if (ev.key.k == XK_greater) data.main_draw.PressUR(14, data.board, ev.button.drw);
    else if (ev.key.k == XK_minus) data.main_draw.PressLS(21, data.board, ev.button.drw);
    else if (ev.key.k == XK_equal) data.main_draw.PressLS(22, data.board, ev.button.drw);
    else if (ev.key.k == XK_underscore) data.main_draw.PressLS(26, data.board, ev.button.drw);
    else if (ev.key.k == XK_plus) data.main_draw.PressLS(27, data.board, ev.button.drw);
    else if (ev.key.k == XK_Delete) data.main_draw.PressLS(7, data.board, ev.button.drw);
    else if (ev.key.k == XK_F8) data.main_draw.PressLS(24, data.board, ev.button.drw);
    else if (ev.key.k == XK_l || ev.key.k == XK_L) data.main_draw.PressLS(6, data.board, ev.button.drw);
#ifndef __MENUET__
    else if (ev.key.k == XK_F2) data.main_draw.PressLS(25, data.board, ev.button.drw);
#endif
    else if (ev.key.k == XK_s || ev.key.k == XK_S) data.main_draw.PressLS(28, data.board, ev.button.drw);
    else if (ev.key.k == XK_slash || ev.key.k == XK_question) data.GoToCurMove(ev.button.drw);
    else if (ev.key.k == XK_n || ev.key.k == XK_N) data.NewGame(ev.button.drw);
    else if (ev.key.k == XK_t || ev.key.k == XK_T) data.RotateBoard(ev.button.drw);
    else if (ev.key.k == XK_r || ev.key.k == XK_R) data.PlayerPress(0, ev.button.drw);
    else if (ev.key.k == XK_b || ev.key.k == XK_B) data.PlayerPress(1, ev.button.drw);
    else if (ev.key.k == XK_f || ev.key.k == XK_F)
    {
      int w, h;
      ev.button.drw->GetSize(w, h);
      ev.button.drw->CloseDraw();
      if (DuplicateProcess() == 0)
      {
        ev.button.drw->ResReinit(w, h);
        data.board.EraseHistory();
      }
    }
    ev.button.drw->CloseDraw();
    break;
  case TGraphDraw::event::close:
    ret = 1;
    break;
  }
  return ret;
}

#ifndef __MENUET__
int main(int argc, char **argv)
{
  randomize();
  THistory::InitHFile(argv[0]);
  TMainData data(-1);
  if (argv && argc >= 2)
  {
    int i, kx = 1;
    for (i = 1; i < argc; i++)
    {
      if (kx == 1 && argv[i][0] == '-')
      {
        if (strcmp(argv[i], "--") == 0) kx = 0;
        else if (strcmp(argv[i], "-ssf") == 0) ssf = 1;
        else if (strncmp(argv[i], "-save", 5) == 0)
        {
          int j = 5;
          if (argv[i][j])
          {
            if (argv[i][j] != '=' || !argv[i][j+1])
            {
              data.main_draw.def_savekind = atoi(argv[i] + j);
              while (argv[i][j] && argv[i][j] != '=') j++;
              if (argv[i][j] != '=' || !argv[i][j+1]) continue;
            }
            data.main_draw.def_savefile = argv[i] + j + 1;
          }
        }
        else printf("Checkers: Invalid key %s\n", argv[i]);
      }
      else if (kx == 0 || kx == 1)
      {
        data.main_draw.play.MsgOpenFile(argv[i], -1);
      }
    }
  }
  data.InitDef();
  TMainGraphDraw graph(
#ifdef BUILD_RUS
  "˜ èª¨"
#else
  "Checkers"
#endif
  );
  TTimerDraw timer_draw(&data.board, &graph);
  data.player.draw = TTimerDraw::draw; data.player.data = &timer_draw;
  graph.evfunc = TMainData::EventFunction; graph.data = &data;
  graph.SetAboutInfo(1);
  graph.Run(TGraphDraw::button_down_mask | TGraphDraw::button_up_mask |
            TGraphDraw::key_down_mask | TGraphDraw::mouse_drag_mask,
            450 + 100 * ssf, 528);
  return 0;
}
#else

TMainData* mdata;
TMainGraphDraw* graph;

bool MenuetOnStart(TStartData &me_start, TThreadData)
{
	mdata = new TMainData(-1);
	graph = new TMainGraphDraw;
	randomize();
	mdata->InitDef();
	static TTimerDraw timer_draw(&mdata->board, graph);
	mdata->player.draw = TTimerDraw::draw; mdata->player.data = &timer_draw;
	graph->data = mdata;
	me_start.WinData.Title =
#ifdef BUILD_RUS
	"˜ èª¨"
#else
	"Checkers"
#endif
	;
	me_start.Width = 450 + 100*ssf;
	me_start.Height = 528;
	return true;
}
bool MenuetOnClose(TThreadData)
{
	delete mdata;
	delete graph;
	return true;
}
int MenuetOnIdle(TThreadData)
{return -1;}
void MenuetOnSize(int window_rect[], TThreadData)
{mdata->board.Resize(window_rect[2]-window_rect[0], window_rect[3]-window_rect[1]);}
void MenuetOnKeyPress(TThreadData)
{
	TGraphDraw::event ev;
	ev.type = TGraphDraw::event::key_down;
	ev.any.drw = graph;
	ev.key.k = GetKey();
	mdata->EventFunction(ev);
}
void MenuetOnDraw(void)
{
	TGraphDraw::event ev;
	ev.type = TGraphDraw::event::draw;
	ev.any.drw = graph;
	mdata->EventFunction(ev);
}
void MenuetOnMouse(TThreadData)
{
	short x,y;
	GetMousePosition(x,y);
	int m = GetMouseButton() & 1;
	static int mprev = 0;
	if (m == mprev)
		return;
	mprev = m;
	TGraphDraw::event ev;
	ev.type = m ? TGraphDraw::event::button_down : TGraphDraw::event::button_up;
	ev.any.drw = graph;
	ev.button.n = 1;
	ev.button.x = x;
	ev.button.y = y;
	mdata->EventFunction(ev);
}
#endif
