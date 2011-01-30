#ifndef _INCLUDE_DOS_MOUSE
#define _INCLUDE_DOS_MOUSE

#include <dos.h>

inline int GetMouseStatus()
{
  int a, b;
  _AX = 0;
  geninterrupt(0x33);
  a = _AX; b = _BX;
  if (!a) return 0;
  else return b;
}

class TCursorVisible
{
public:
  TCursorVisible(int s = 1) : visible(1) {if (!s) Hide();}
  ~TCursorVisible() {Show();}

  int IsShowed() const {return visible;}
  int IsHidden() const {return !visible;}
  int Show()
  {
    if (!visible) {_AX = 1; geninterrupt(0x33); visible = 1; return 1;}
    else return 0;
  }
  int Hide()
  {
    if (visible) {_AX = 2; geninterrupt(0x33); visible = 0; return 1;}
    else return 0;
  }
  int Set(int v) {return v ? Show() : Hide();}
private:
  int visible;
public:
  class T_C
  {
  public:
    ~T_C() {Hide();}

    int IsShowed() const {return visible;}
    int IsHidden() const {return !visible;}
    int Show()
    {
      if (!visible) {_AX = 1; geninterrupt(0x33); visible = 1; return 1;}
      else return 0;
    }
    int Hide()
    {
      if (visible) {_AX = 2; geninterrupt(0x33); visible = 0; return 1;}
      else return 0;
    }
    int Set(int v) {return v ? Show() : Hide();}
  private:
    T_C() : visible(0) {}
    friend class TCursorVisible;

    int visible;
  };

  static T_C C;
};

extern int MouseStatus;

inline void SetShowCursor(int show)
{
  if (MouseStatus)
  {
    _AX = show ? 1 : 2;
    geninterrupt(0x33);
  }
}

inline void ShowCursor()
{
  if (MouseStatus)
  {
    _AX = 1;
    geninterrupt(0x33);
  }
}

inline void HideCursor()
{
  if (MouseStatus)
  {
    _AX = 2;
    geninterrupt(0x33);
  }
}

inline int PosCursor(int &x, int &y)
{
  if (MouseStatus)
  {
    int xx, yy, r;
    _AX = 3;
    geninterrupt(0x33);
    xx = _CX; yy = _DX;
    r = _BX;
    x = xx; y = yy;
    return r;
  }
  else return 0;
}

inline int SetPosCursor(int x, int y)
{
  if (MouseStatus)
  {
    _AX = 4; _CX = x; _DX = y;
    geninterrupt(0x33);
    return _BX;
  }
  else return 0;
}

inline int GetButtonDown(int n, int &x, int &y)
{
  if (MouseStatus)
  {
    int xx, yy, r;
    _AX = 5; _BX = n;
    geninterrupt(0x33);
    xx = _CX; yy = _DX;
    r = _BX;
    x = xx; y = yy;
    return r;
  }
  else return 0;
}

inline int GetButtonUp(int n, int &x, int &y)
{
  if (MouseStatus)
  {
    int xx, yy, r;
    _AX = 6; _BX = n;
    geninterrupt(0x33);
    xx = _CX; yy = _DX;
    r = _BX;
    x = xx; y = yy;
    return r;
  }
  else return 0;
}

inline void BoundCursorX(int x1, int x2)
{
  if (MouseStatus)
  {
    _AX = 7; _CX = x1; _DX = x2;
    geninterrupt(0x33);
  }
}

inline void BoundCursorY(int y1, int y2)
{
  if (MouseStatus)
  {
    _AX = 8; _CX = y1; _DX = y2;
    geninterrupt(0x33);
  }
}

int MouseStatus = GetMouseStatus();
TCursorVisible::T_C TCursorVisible::C;

#endif  //_INCLUDE_DOS_MOUSE
