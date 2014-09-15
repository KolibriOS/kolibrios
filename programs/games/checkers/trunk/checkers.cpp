//#define LANG_RUS
#define NDEBUG
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// sprintf is too heavy
int itoa(char* dst, int number)
{
	int result = 0;
	if (number < 0) {
		result++;
		*dst++ = '-';
		number = -number;
	}
	char digits[16];
	int ndigits = 0;
	do {
		digits[ndigits++] = number % 10;
		number /= 10;
	} while (number);
	result += ndigits;
	for (; ndigits--; )
		*dst++ = digits[ndigits] + '0';
	return result;
}

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
#ifndef NO_FILES
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
#ifndef NO_FILES
  static const char *const search[];
  static int AnalizeKind(FILE *f, const int w[], const char *smb = 0);
  static int ReadFileTo(FILE *f, char c);
  static int ReadFileTo(FILE *f, const char *c);
  static int ReadTheText(FILE *f, const char *s);
#endif
};

#ifdef LANG_RUS //Pending Russian Translations
    #define CHECKERS_CANT_OPEN_STR "\nШашки: Не могу открыть файл \"%s\".\n"
    #define CHECKERS_FILE_EMPTY_STR "\nШашки: Файл \"%s\" пуст.\n"
    #define CHECKERS_INVALID_FILE_STR "\nШашки: Файл \"%s\" ошибочный.\n"
    #define FILE_WRONG_TYPE_STR "\nШашки: Файл \"%s\" имеет неверный тип.\n"
    #define ERROR_OPENING_FILE_STR "\nШашки: Ошибка открытия файла \"%s\".\n"
    #define FILE_HAS_NO_GAMES_STR "\nШашки: Файл \"%s\" не содержит игр.\n"
    #define CANT_OPEN_FILE_STR "\nШашки: Не могу открыть файл \"%s\" на запись.\n"
    #define CHECKERS_STR "Шашки"
    #define FILE_SAVED_STR "\nШашки: Файл \"%s\" сохранён.\n"
    #define ERROR_SAVING_FILE_STR "\nШашки: Ошибка сохранения файла \"%s\".\n"
    #define NOT_ENOUGH_MEM_STR "\nШашки: Не достаточно памяти для сохранения файла \"%s\".\n"
    #define KIND_EQ2 "kind = 2\n"
    #define END_STR "[end]"
    #define PLAYV11_STR "[play_v1.1]"
    #define CHECKERS_PLAY_STR "checkers-play:text\n"
    #define CHECKERS_BIN_STR "checkers-play:bin_1.0\n"
    #define BLUE_STR "blue: "
    #define RED_STR "red: "
    #define INPUT_STR "input"
    #define COMPUTER_STR "computer"
    #define NEW_GAME_STR "new game"
    #define LIST_STR "list"
    #define DELETE_STR "delete"
    #define CLEAR_STR "clear"
    #define SAVE_STR "save"
    #define ROTATE_BOARD_STR "rotate board"
    #define EXIT_STR "exit"
    #define CHECKERS_INVALID_STR "Checkers: Invalid key %s\n"

#ifndef NO_FILES
const char *const TPlayArray::search[] =
  {"checkers-", "play:", "text", "bin_1.0\n", "history_", "1.0", "1.1"};
#endif

#else /*For all other languages except RUS*/
    #define CHECKERS_CANT_OPEN_STR "\nCheckers: Can't open the file \"%s\".\n"
    #define CHECKERS_FILE_EMPTY_STR "\nCheckers: The file \"%s\" is empty.\n"
    #define CHECKERS_INVALID_FILE_STR "\nCheckers: Invalid file \"%s\".\n"
    #define FILE_WRONG_TYPE_STR "\nCheckers: The file \"%s\" has the wrong type.\n"
    #define ERROR_OPENING_FILE_STR "\nCheckers: Error opening the file \"%s\".\n"
    #define FILE_HAS_NO_GAMES_STR "\nCheckers: File \"%s\" doesn't contain games.\n"
    #define CANT_OPEN_FILE_STR "\nCheckers: Can't open the file \"%s\" to write.\n"
    #define NOT_ENOUGH_MEM_STR "\nCheckers: Not enough memory to save the file \"%s\".\n"
    #define ERROR_SAVING_FILE_STR "\nCheckers: Error saving the file \"%s\".\n"
    #define FILE_SAVED_STR "\nCheckers: File \"%s\" saved.\n"
    #define CHECKERS_STR "Checkers"
    #define KIND_EQ2 "kind = 2\n"
    #define END_STR "[end]"
    #define PLAYV11_STR "[Play_v1.1]"
    #define CHECKERS_PLAY_STR "checkers-play:text\n"
    #define CHECKERS_BIN_STR "checkers-play:bin_1.0\n"
    #define BLUE_STR "blue: "
    #define RED_STR "red: "
    #define INPUT_STR "input"
    #define COMPUTER_STR "computer"
    #define NEW_GAME_STR "new game"
    #define LIST_STR "list"
    #define DELETE_STR "delete"
    #define CLEAR_STR "clear"
    #define SAVE_STR "save"
    #define ROTATE_BOARD_STR "rotate board"
    #define EXIT_STR "exit"
    #define CHECKERS_INVALID_STR "Checkers: Invalid key %s\n"

#ifndef NO_FILES
const char *const TPlayArray::search[] =
     {"checkers-", "play:", "text", "bin_1.0\n", "history_", "1.0", "1.1"};
#endif

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

#ifndef NO_FILES
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
    printf(KIND_EQ2);
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
      if (feof(f)) strcpy(word, END_STR);
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
	char end_literal[] = END_STR;
	char playv11_literal[] = PLAYV11_STR;

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
        if (strskipref(word, end_literal)) break;
        else if (strskipref(word, playv11_literal)) kind = 1;
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
      printf(CHECKERS_CANT_OPEN_STR, name);
      return 0;
    }
    else if (r == -2)
    {
      printf(CHECKERS_FILE_EMPTY_STR, name);
      return 0;
    }
    else if (r == -3)
    {
      printf(CHECKERS_INVALID_FILE_STR, name);
      return 0;
    }
    else if (r == -4) no_games = 1;
    else if (r == -10)
    {
      printf(FILE_WRONG_TYPE_STR, name);
      return 0;
    }
    else
    {
      printf(ERROR_OPENING_FILE_STR, name);
      return 0;
    }
  }
  if (!no_games && nplay > n0) return 1;
  else
  {
    printf(FILE_HAS_NO_GAMES_STR, name);
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

    fprintf(f, CHECKERS_PLAY_STR);
    int i0 = num, i;
    if (num < 0) {i0 = 0; num = nplay-1;}
    for (i = i0; i <= num; i++) if (play[i].GetN() > 0)
    {
      PlayWrite::PMv pmv;
      if (play[i].GetPos(pmv.pos, 0) < 0) return -9;
      char *str = new char[10 + NUM_CELL];
      if (!str) return -5;
      pmv.pos.Write(str, 1);
      fprintf(f, "\n"PLAYV11_STR"#%d  %s\n", i - i0 + 1, str);
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
    if (kind == 3) fprintf(f, CHECKERS_BIN_STR);

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
      printf(CANT_OPEN_FILE_STR, name);
    }
    else if (r == -5)
    {
      printf(NOT_ENOUGH_MEM_STR, name);
    }
    else if (r == -10)
    {
      printf(FILE_WRONG_TYPE_STR, name);
    }
    else
    {
      printf(ERROR_SAVING_FILE_STR, name);
    }
    return 0;
  }
  else
  {
    printf(FILE_SAVED_STR, name);
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
  strcpy(str, c ? BLUE_STR : RED_STR);
  if (!p) strcat(str, INPUT_STR);
  else strcat(str, COMPUTER_STR);
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
  static char newgame_literal[] = NEW_GAME_STR;
  static char list_literal[] = LIST_STR;
  static char delete_literal[] = DELETE_STR;
  static char clear_literal[] = CLEAR_STR;
  static char save_literal[] = SAVE_STR;
  static char rotateboard_literal[] = ROTATE_BOARD_STR;
  static char exit_literal[] = EXIT_STR;

  static char one_literal[] = "1";
  static char dash_literal[] = "-";
  static char plus_literal[] = "+";
  static char leftshift_literal[] = "<<";
  static char rightshift_literal[] = ">>";
  static char lessthan_literal[] = "<";
  static char greaterthan_literal[] = ">";
  static char xor_literal[] = "^";

  button.Add(1, 80, 22, newgame_literal);
  button.Add(6, 60, 22, list_literal);
  button.Add(7, 60, 22, delete_literal);
  play_list = new TMultiButton(20);
  play_list->a.Add(26, 20, 22, one_literal);
  play_list->a.Add(21, 20, 22, dash_literal);
  play_list->a.Add(23, 37, 22, play_num[0], -2);
  play_list->a.Add(22, 20, 22, plus_literal);
  play_list->a.Add(27, 37, 22, play_num[1]);
  play_list->SetDefW();
  button.Add(play_list);
  button.Add(24, 50, 22, clear_literal);
#ifndef NO_FILES
  button.Add(25, 50, 22, save_literal);
#endif
  button.Add(2, 120, 22, player_str[0]);
  button.Add(3, 120, 22, player_str[1]);
  button.Add(4, 110, 22, rotateboard_literal);
  undo_redo = new TMultiButton(10);
  undo_redo->a.Add(11, 27, 22, leftshift_literal);
  undo_redo->a.Add(12, 20, 22, lessthan_literal);
  undo_redo->a.Add(15, 20, 22, xor_literal);
  undo_redo->a.Add(13, 20, 22, greaterthan_literal);
  undo_redo->a.Add(14, 27, 22, rightshift_literal);
  undo_redo->SetDefW();
  button.Add(undo_redo);
  button.Add(5, 60, 22, exit_literal);
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
#ifndef NO_FILES
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
    if (cur_play >= 0) itoa(play_num[0], cur_play + 1);
    itoa(play_num[1], play.GetNPlay());
    thick = (cur_play <= 0) ? 0 : 3;
    txb = static_cast<TextButton*>(play_list->a.GetButton(21));
    if (txb) txb->thick = thick;
    txb = static_cast<TextButton*>(play_list->a.GetButton(26));
    if (txb) txb->thick = thick;
    thick = (cur_play >= play.GetNPlay() - 1) ? 0 : 3;
    txb = static_cast<TextButton*>(play_list->a.GetButton(22));
    if (txb) txb->thick = thick;
    txb = static_cast<TextButton*>(play_list->a.GetButton(27));
    if (txb) txb->thick = thick;
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
#ifndef NO_FILES
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
#ifndef NO_FILES
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

int main(int argc, char **argv)
{
  randomize();
#ifndef NO_FILES
  THistory::InitHFile(argv[0]);
#endif
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

        else printf(CHECKERS_INVALID_STR, argv[i]);
      }
      else if (kx == 0 || kx == 1)
      {
#ifndef NO_FILES
        data.main_draw.play.MsgOpenFile(argv[i], -1);
#endif
      }
    }
  }
  data.InitDef();
  TMainGraphDraw graph(CHECKERS_STR);
  TTimerDraw timer_draw(&data.board, &graph);
  data.player.draw = TTimerDraw::draw; data.player.data = &timer_draw;
  graph.evfunc = TMainData::EventFunction; graph.data = &data;
  graph.SetAboutInfo(1);
  graph.Run(TGraphDraw::button_down_mask | TGraphDraw::button_up_mask |
            TGraphDraw::key_down_mask | TGraphDraw::mouse_drag_mask,
            450 + 100 * ssf, 528);
  return 0;
}
