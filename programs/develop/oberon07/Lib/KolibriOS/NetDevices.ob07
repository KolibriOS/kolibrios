(*
    Copyright 2017 Anton Krotov

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

MODULE NetDevices;

IMPORT sys := SYSTEM, K := KOSAPI;


CONST

  //net devices types

  LOOPBACK*        = 0;
  ETH*             = 1;
  SLIP*            = 2;

  //Link status

  LINK_DOWN*       = 0;
  LINK_UNKNOWN*    = 1;
  LINK_FD*         = 2; //full duplex flag
  LINK_10M*        = 4;
  LINK_100M*       = 8;
  LINK_1G*         = 12;


TYPE

  DEVICENAME* = ARRAY 64 OF CHAR;


PROCEDURE Number* (): INTEGER;
  RETURN K.sysfunc2(74, -1)
END Number;


PROCEDURE Type* (num: INTEGER): INTEGER;
  RETURN K.sysfunc2(74, num * 256)
END Type;


PROCEDURE Name* (num: INTEGER; VAR name: DEVICENAME): BOOLEAN;
VAR err: BOOLEAN;
BEGIN
  err := K.sysfunc3(74, num * 256 + 1, sys.ADR(name[0])) = -1;
  IF err THEN
    name := ""
  END
  RETURN ~err
END Name;


PROCEDURE Reset* (num: INTEGER): BOOLEAN;
  RETURN K.sysfunc2(74, num * 256 + 2) # -1
END Reset;


PROCEDURE Stop* (num: INTEGER): BOOLEAN;
  RETURN K.sysfunc2(74, num * 256 + 3) # -1
END Stop;


PROCEDURE Pointer* (num: INTEGER): INTEGER;
  RETURN K.sysfunc2(74, num * 256 + 4)
END Pointer;


PROCEDURE SentPackets* (num: INTEGER): INTEGER;
  RETURN K.sysfunc2(74, num * 256 + 6)
END SentPackets;


PROCEDURE ReceivedPackets* (num: INTEGER): INTEGER;
  RETURN K.sysfunc2(74, num * 256 + 7)
END ReceivedPackets;


PROCEDURE SentBytes* (num: INTEGER; VAR hValue: INTEGER): INTEGER;
  RETURN K.sysfunc22(74, num * 256 + 8, hValue)
END SentBytes;


PROCEDURE ReceivedBytes* (num: INTEGER; VAR hValue: INTEGER): INTEGER;
  RETURN K.sysfunc22(74, num * 256 + 9, hValue)
END ReceivedBytes;


PROCEDURE LinkStatus* (num: INTEGER): INTEGER;
  RETURN K.sysfunc2(74, num * 256 + 10)
END LinkStatus;


END NetDevices.