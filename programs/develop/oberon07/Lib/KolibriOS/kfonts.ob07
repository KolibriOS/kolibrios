(*
    Copyright 2016, 2018 Anton Krotov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*)

MODULE kfonts;

IMPORT sys := SYSTEM, File, KOSAPI;

CONST

  MIN_FONT_SIZE = 8;
  MAX_FONT_SIZE = 46;

  bold            *=   1;
  italic          *=   2;
  underline       *=   4;
  strike_through  *=   8;
  smoothing       *=  16;
  bpp32           *=  32;

TYPE

  Glyph = RECORD
    base: INTEGER;
    xsize, ysize: INTEGER;
    width: INTEGER
  END;

  TFont_desc = RECORD

    data, size, font, char_size, width, height, font_size, mem, mempos: INTEGER;
    glyphs: ARRAY 4, 256 OF Glyph

  END;

  TFont* = POINTER TO TFont_desc;


PROCEDURE [stdcall] zeromem(size, adr: INTEGER);
BEGIN
  sys.CODE(057H, 08BH, 07DH, 00CH, 08BH, 04DH, 008H, 033H, 0C0H, 09CH, 0FCH, 0F3H, 0ABH, 09DH, 05FH)
END zeromem;

PROCEDURE pset(buf, x, y, color: INTEGER; bpp32: BOOLEAN);
VAR xsize, ysize: INTEGER;
BEGIN
  sys.GET(buf, xsize);
  sys.GET(buf + 4, ysize);
  INC(buf, 8);
  IF (0 <= x) & (x < xsize) & (0 <= y) & (y < ysize) THEN
    IF bpp32 THEN
      sys.PUT(buf + 4 * (xsize * y + x), color)
    ELSE
      sys.MOVE(sys.ADR(color), buf + 3 * (xsize * y + x), 3)
    END
  END
END pset;

PROCEDURE pget(buf, x, y: INTEGER; bpp32: BOOLEAN): INTEGER;
VAR xsize, ysize, color: INTEGER;
BEGIN
  sys.GET(buf, xsize);
  sys.GET(buf + 4, ysize);
  INC(buf, 8);
  IF (0 <= x) & (x < xsize) & (0 <= y) & (y < ysize) THEN
    IF bpp32 THEN
      sys.GET(buf + 4 * (xsize * y + x), color)
    ELSE
      sys.MOVE(buf + 3 * (xsize * y + x), sys.ADR(color), 3)
    END
  END
  RETURN color
END pget;

PROCEDURE getrgb(color: INTEGER; VAR r, g, b: INTEGER);
BEGIN
  b := LSR(LSL(color, 24), 24);
  g := LSR(LSL(color, 16), 24);
  r := LSR(LSL(color,  8), 24);
END getrgb;

PROCEDURE rgb(r, g, b: INTEGER): INTEGER;
  RETURN b + LSL(g, 8) + LSL(r, 16)
END rgb;

PROCEDURE create_glyph(VAR Font: TFont_desc; VAR glyph: Glyph; xsize, ysize: INTEGER);
BEGIN
  glyph.base := Font.mempos;
  glyph.xsize := xsize;
  glyph.ysize := ysize;
  Font.mempos := Font.mempos + xsize * ysize
END create_glyph;

PROCEDURE getpix(Font: TFont_desc; n, x, y, xsize: INTEGER): CHAR;
VAR res: CHAR;
BEGIN
  sys.GET(Font.mem + n + x + y * xsize, res)
  RETURN res
END getpix;

PROCEDURE setpix(VAR Font: TFont_desc; n, x, y, xsize: INTEGER; c: CHAR);
BEGIN
  sys.PUT(Font.mem + n + x + y * xsize, c)
END setpix;

PROCEDURE smooth(VAR Font: TFont_desc; n, xsize, ysize: INTEGER);
VAR x, y: INTEGER;
BEGIN
  FOR y := 1 TO ysize - 1 DO
    FOR x := 1 TO xsize - 1 DO
      IF (getpix(Font, n, x, y, xsize) = 1X) & (getpix(Font, n, x - 1, y - 1, xsize) = 1X) &
         (getpix(Font, n, x - 1, y, xsize) = 0X) & (getpix(Font, n, x, y - 1, xsize) = 0X) THEN
        setpix(Font, n, x - 1, y, xsize, 2X);
        setpix(Font, n, x, y - 1, xsize, 2X)
      END;
      IF (getpix(Font, n, x, y, xsize) = 0X) & (getpix(Font, n, x - 1, y - 1, xsize) = 0X) &
         (getpix(Font, n, x - 1, y, xsize) = 1X) & (getpix(Font, n, x, y - 1, xsize) = 1X) THEN
        setpix(Font, n, x, y, xsize, 2X);
        setpix(Font, n, x - 1, y - 1, xsize, 2X)
      END
    END
  END
END smooth;

PROCEDURE _bold(VAR Font: TFont_desc; src, dst, src_xsize, dst_xsize, n: INTEGER);
VAR i, j, k: INTEGER; pix: CHAR;
BEGIN
  FOR i := 0 TO src_xsize - 1 DO
    FOR j := 0 TO Font.height - 1 DO
      pix := getpix(Font, src, i, j, src_xsize);
      IF pix = 1X THEN
        FOR k := 0 TO n DO
          setpix(Font, dst, i + k, j, dst_xsize, pix)
        END
      END
    END
  END
END _bold;

PROCEDURE make_glyph(VAR Font: TFont_desc; c: INTEGER);
VAR ptr, i, j, max, x, y: INTEGER; s: SET; eoc: BOOLEAN;
    glyph: Glyph; pix: CHAR; bold_width: INTEGER;
BEGIN
  create_glyph(Font, glyph, Font.width, Font.height);
  x := 0;
  y := 0;
  max := 0;
  ptr := Font.font + Font.char_size * c;
  eoc := FALSE;
  REPEAT
    sys.GET(ptr, s);
    INC(ptr, 4);
    FOR i := 0 TO 31 DO
      IF ~eoc THEN
        IF i IN s THEN
          setpix(Font, glyph.base, x, y, Font.width, 1X);
          IF x > max THEN
            max := x
          END
        ELSE
          setpix(Font, glyph.base, x, y, Font.width, 0X)
        END
      END;
      INC(x);
      IF x = Font.width THEN
        x := 0;
        INC(y);
        eoc := eoc OR (y = Font.height)
      END
    END
  UNTIL eoc;
  IF max = 0 THEN
    max := Font.width DIV 3
  END;

  glyph.width := max;
  smooth(Font, glyph.base, glyph.xsize, glyph.ysize);
  Font.glyphs[0, c] := glyph;

  bold_width := 1;

  create_glyph(Font, glyph, Font.width + bold_width, Font.height);
  _bold(Font, Font.glyphs[0, c].base, glyph.base, Font.glyphs[0, c].xsize, glyph.xsize, bold_width);
  smooth(Font, glyph.base, glyph.xsize, glyph.ysize);
  glyph.width := max + bold_width;
  Font.glyphs[1, c] := glyph;

  create_glyph(Font, glyph, Font.width + (Font.height - 1) DIV 3, Font.height);
  FOR i := 0 TO Font.glyphs[0, c].xsize - 1 DO
    FOR j := 0 TO Font.height - 1 DO
      pix := getpix(Font, Font.glyphs[0, c].base, i, j, Font.glyphs[0, c].xsize);
      IF pix = 1X THEN
        setpix(Font, glyph.base, i + (Font.height - 1 - j) DIV 3, j, glyph.xsize, pix)
      END
    END
  END;
  smooth(Font, glyph.base, glyph.xsize, glyph.ysize);
  glyph.width := max;
  Font.glyphs[2, c] := glyph;

  create_glyph(Font, glyph, Font.width + (Font.height - 1) DIV 3 + bold_width, Font.height);
  _bold(Font, Font.glyphs[2, c].base, glyph.base, Font.glyphs[2, c].xsize, glyph.xsize, bold_width);
  smooth(Font, glyph.base, glyph.xsize, glyph.ysize);
  glyph.width := max + bold_width;
  Font.glyphs[3, c] := glyph;

END make_glyph;

PROCEDURE OutChar(Font: TFont_desc; c: INTEGER; x, y: INTEGER; buf: INTEGER; bpp32, smoothing: BOOLEAN; color, style: INTEGER): INTEGER;
VAR i, x0, y0, xsize, mem, xmax: INTEGER; r, g, b, r0, g0, b0: INTEGER; ch: CHAR; glyph: Glyph;
BEGIN
  x0 := x;
  y0 := y;
  style := style MOD 4;
  glyph := Font.glyphs[style, c];
  xsize := glyph.xsize;
  xmax := x0 + xsize;
  mem := Font.mem + glyph.base;
  getrgb(color, r0, g0, b0);
  FOR i := mem TO mem + xsize * Font.height - 1 DO
    sys.GET(i, ch);
    IF ch = 1X THEN
      pset(buf, x, y, color, bpp32);
    ELSIF (ch = 2X) & smoothing THEN
      getrgb(pget(buf, x, y, bpp32), r, g, b);
      r := (r * 3 + r0) DIV 4;
      g := (g * 3 + g0) DIV 4;
      b := (b * 3 + b0) DIV 4;
      pset(buf, x, y, rgb(r, g, b), bpp32)
    END;
    INC(x);
    IF x = xmax THEN
      x := x0;
      INC(y)
    END
  END
  RETURN glyph.width
END OutChar;

PROCEDURE hline(buf, x, y, width, color: INTEGER; bpp32: BOOLEAN);
VAR i: INTEGER;
BEGIN
  FOR i := x TO x + width - 1 DO
    pset(buf, i, y, color, bpp32)
  END
END hline;

PROCEDURE TextWidth*(Font: TFont; str, length, params: INTEGER): INTEGER;
VAR res: INTEGER; c: CHAR;
BEGIN
  res := 0;
  params := params MOD 4;
  IF Font # NIL THEN
    sys.GET(str, c);
    WHILE (length > 0) OR (length = -1) & (c # 0X) DO
      INC(str);
      res := res + Font.glyphs[params, ORD(c)].width;
      IF length > 0 THEN
        DEC(length)
      END;
      IF length # 0 THEN
        sys.GET(str, c)
      END
    END
  END
  RETURN res
END TextWidth;

PROCEDURE TextHeight*(Font: TFont): INTEGER;
VAR res: INTEGER;
BEGIN
  IF Font # NIL THEN
    res := Font.height
  ELSE
    res := 0
  END
  RETURN res
END TextHeight;

PROCEDURE TextClipLeft(Font: TFont; str, length, params: INTEGER; VAR x: INTEGER): INTEGER;
VAR x1: INTEGER; c: CHAR;
BEGIN
  params := params MOD 4;
  sys.GET(str, c);
  WHILE (length > 0) OR (length = -1) & (c # 0X) DO
    INC(str);
    x1 := x;
    x := x + Font.glyphs[params, ORD(c)].width;
    IF x > 0 THEN
      length := 0;
    END;
    IF length > 0 THEN
      DEC(length)
    END;
    IF length # 0 THEN
      sys.GET(str, c)
    END
  END;
  x := x1
  RETURN str - 1
END TextClipLeft;

PROCEDURE TextOut*(Font: TFont; canvas, x, y, str, length, color, params: INTEGER);
VAR width, xsize, ysize, str1, n: INTEGER; c: CHAR; bpp32, smoothing, underline, strike: BOOLEAN;
BEGIN
  IF Font # NIL THEN
    sys.GET(canvas,     xsize);
    sys.GET(canvas + 4, ysize);
    IF (y <= -TextHeight(Font)) OR (y >= ysize) THEN
      length := 0
    END;
    IF length # 0 THEN
      smoothing := 4 IN BITS(params);
      bpp32 := 5 IN BITS(params);
      underline := 2 IN BITS(params);
      strike := 3 IN BITS(params);
      str1 := TextClipLeft(Font, str, length, params, x);
      n := str1 - str;
      str := str1;
      IF length >= n THEN
        length := length - n
      END;
      sys.GET(str, c)
    END;
    WHILE (length > 0) OR (length = -1) & (c # 0X) DO
      INC(str);
      width := OutChar(Font^, ORD(c), x, y, canvas, bpp32, smoothing, color, params);
      IF strike THEN
        hline(canvas, x + ORD(1 IN BITS(params)) * ((Font.height DIV 2) DIV 3), y + Font.height DIV 2, width + 2, color, bpp32)
      END;
      IF underline THEN
        hline(canvas, x, y + Font.height - 1, width + 2, color, bpp32)
      END;
      x := x + width;
      IF x > xsize THEN
        length := 0
      END;
      IF length > 0 THEN
        DEC(length)
      END;
      IF length # 0 THEN
        sys.GET(str, c)
      END
    END
  END
END TextOut;

PROCEDURE SetSize*(_Font: TFont; font_size: INTEGER): BOOLEAN;
VAR temp, offset, fsize, i, memsize, mem: INTEGER;
    c: CHAR; Font, Font2: TFont_desc;
BEGIN
  offset := -1;
  IF (MIN_FONT_SIZE <= font_size) & (font_size <= MAX_FONT_SIZE) & (_Font # NIL) THEN
    Font := _Font^;
    Font2 := Font;
    temp := Font.data + (font_size - 8) * 4;
    IF (Font.data <= temp) & (temp <= Font.size + Font.data - 4) THEN
      sys.GET(temp, offset);
      IF offset # -1 THEN
        Font.font_size := font_size;
        INC(offset, 156);
        offset := offset + Font.data;
        IF (Font.data <= offset) & (offset <= Font.size + Font.data - 4) THEN
          sys.GET(offset, fsize);
          IF fsize > 256 + 6 THEN
            temp := offset + fsize - 1;
            IF (Font.data <= temp) & (temp <= Font.size + Font.data - 1) THEN
              sys.GET(temp, c);
              IF c # 0X THEN
                Font.height := ORD(c);
                DEC(temp);
                sys.GET(temp, c);
                IF c # 0X THEN
                  Font.width := ORD(c);
                  DEC(fsize, 6);
                  Font.char_size := fsize DIV 256;
                  IF fsize MOD 256 # 0 THEN
                    INC(Font.char_size)
                  END;
                  IF Font.char_size > 0 THEN
                    Font.font := offset + 4;
                    Font.mempos := 0;
                    memsize := (Font.width + 10) * Font.height * 1024;
                    mem := Font.mem;
                    Font.mem := KOSAPI.sysfunc3(68, 12, memsize);
                    IF Font.mem # 0 THEN
                      IF mem # 0 THEN
                        mem := KOSAPI.sysfunc3(68, 13, mem)
                      END;
                      zeromem(memsize DIV 4, Font.mem);
                      FOR i := 0 TO 255 DO
                        make_glyph(Font, i)
                      END
                    ELSE
                      offset := -1
                    END
                  ELSE
                    offset := -1
                  END
                ELSE
                  offset := -1
                END
              ELSE
                offset := -1
              END
            ELSE
              offset := -1
            END
          ELSE
            offset := -1
          END
        ELSE
          offset := -1
        END
      END;
    ELSE
      offset := -1
    END;
    IF offset # -1 THEN
      _Font^ := Font
    ELSE
      _Font^ := Font2
    END
  END
  RETURN offset # -1
END SetSize;

PROCEDURE Enabled*(Font: TFont; font_size: INTEGER): BOOLEAN;
VAR offset, temp: INTEGER;
BEGIN
  offset := -1;
  IF (MIN_FONT_SIZE <= font_size) & (font_size <= MAX_FONT_SIZE) & (Font # NIL) THEN
    temp := Font.data + (font_size - 8) * 4;
    IF (Font.data <= temp) & (temp <= Font.size + Font.data - 4) THEN
      sys.GET(temp, offset)
    END
  END
  RETURN offset # -1
END Enabled;

PROCEDURE Destroy*(VAR Font: TFont);
BEGIN
  IF Font # NIL THEN
    IF Font.mem # 0 THEN
      Font.mem := KOSAPI.sysfunc3(68, 13, Font.mem)
    END;
    IF Font.data # 0 THEN
      Font.data := KOSAPI.sysfunc3(68, 13, Font.data)
    END;
    DISPOSE(Font)
  END
END Destroy;

PROCEDURE LoadFont*(file_name: ARRAY OF CHAR): TFont;
VAR Font: TFont; data, size, n: INTEGER;
BEGIN
  data := File.Load(file_name, size);
  IF (data # 0) & (size > 156) THEN
    NEW(Font);
    Font.data := data;
    Font.size := size;
    Font.font_size := 0;
    n := MIN_FONT_SIZE;
    WHILE ~SetSize(Font, n) & (n <= MAX_FONT_SIZE) DO
      INC(n)
    END;
    IF Font.font_size = 0 THEN
      Destroy(Font)
    END
  ELSE
    IF data # 0 THEN
      data := KOSAPI.sysfunc3(68, 13, data)
    END;
    Font := NIL
  END
  RETURN Font
END LoadFont;

END kfonts.