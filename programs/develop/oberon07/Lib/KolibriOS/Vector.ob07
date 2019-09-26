(*
    Copyright 2016 Anton Krotov

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

MODULE Vector;


IMPORT sys := SYSTEM, K := KOSAPI;


TYPE

  DESC_VECTOR = RECORD

    data   : INTEGER;
    count  : INTEGER;
    size   : INTEGER

  END;

  VECTOR* = POINTER TO DESC_VECTOR;

  ANYREC* = RECORD END;

  ANYPTR* = POINTER TO ANYREC;

  DESTRUCTOR* = PROCEDURE (VAR ptr: ANYPTR);


PROCEDURE count* (vector: VECTOR): INTEGER;
BEGIN
  ASSERT(vector # NIL)
  RETURN vector.count
END count;


PROCEDURE push* (vector: VECTOR; value: ANYPTR);
BEGIN
  ASSERT(vector # NIL);
  IF vector.count = vector.size THEN
    vector.data := K.realloc(vector.data, (vector.size + 1024) * 4);
    ASSERT(vector.data # 0);
    vector.size := vector.size + 1024
  END;
  sys.PUT(vector.data + vector.count * 4, value);
  INC(vector.count)
END push;


PROCEDURE get* (vector: VECTOR; idx: INTEGER): ANYPTR;
VAR res: ANYPTR;
BEGIN
  ASSERT(vector # NIL);
  ASSERT( (0 <= idx) & (idx < vector.count) );
  sys.GET(vector.data + idx * 4, res)
  RETURN res
END get;


PROCEDURE put* (vector: VECTOR; idx: INTEGER; value: ANYPTR);
BEGIN
  ASSERT(vector # NIL);
  ASSERT( (0 <= idx) & (idx < vector.count) );
  sys.PUT(vector.data + idx * 4, value)
END put;


PROCEDURE create* (size: INTEGER): VECTOR;
VAR vector: VECTOR;
BEGIN
  NEW(vector);
  IF vector # NIL THEN
    vector.data  := K.malloc(4 * size);
    IF vector.data # 0 THEN
      vector.size  := size;
      vector.count := 0
    ELSE
      DISPOSE(vector)
    END
  END
  RETURN vector
END create;


PROCEDURE def_destructor (VAR any: ANYPTR);
BEGIN
  DISPOSE(any)
END def_destructor;


PROCEDURE destroy* (VAR vector: VECTOR; destructor: DESTRUCTOR);
VAR i: INTEGER;
    any: ANYPTR;
BEGIN
  ASSERT(vector # NIL);
  IF destructor = NIL THEN
    destructor := def_destructor
  END;
  FOR i := 0 TO vector.count - 1 DO
    any := get(vector, i);
    destructor(any)
  END;
  vector.data := K.free(vector.data);
  DISPOSE(vector)
END destroy;


END Vector.