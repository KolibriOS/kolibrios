;;================================================================================================;;
;;//// libio.inc //// (c) mike.dld, 2007-2008 ////////////////////////////////////////////////////;;
;;================================================================================================;;
;;                                                                                                ;;
;; This file is part of Common development libraries (Libs-Dev).                                  ;;
;;                                                                                                ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under the terms of the GNU ;;
;; Lesser General Public License as published by the Free Software Foundation, either version 2.1 ;;
;; of the License, or (at your option) any later version.                                         ;;
;;                                                                                                ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  ;;
;; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  ;;
;; Lesser General Public License for more details.                                                ;;
;;                                                                                                ;;
;; You should have received a copy of the GNU Lesser General Public License along with Libs-Dev.  ;;
;; If not, see <http://www.gnu.org/licenses/>.                                                    ;;
;;                                                                                                ;;
;;================================================================================================;;


O_BINARY = 00000000b
O_READ   = 00000001b
O_WRITE  = 00000010b
O_CREATE = 00000100b
O_SHARE  = 00001000b
O_TEXT   = 00010000b

SEEK_SET = 0
SEEK_CUR = 1
SEEK_END = 2

struct FileDateTime
  union
    time    dd ?
    struct
      sec   db ?
      min   db ?
      hour  db ?
    ends
  ends
  union
    date    dd ?
    struct
      day   db ?
      month db ?
      year  dw ?
    ends
  ends
ends

struct FileInfoBlock
  Function   dd ?
  Position   dd ?
  Flags      dd ?
  Count      dd ?
  Buffer     dd ?
             db ?
  FileName   dd ?
ends

struct FileInfoHeader
  Version    dd ?
  FilesRead  dd ?
  FilesCount dd ?
             rd 5
ends

struct FileInfoA
  Attributes   dd ?
  Flags        dd ?
  DateCreate   FileDateTime
  DateAccess   FileDateTime
  DateModify   FileDateTime
  union
    FileSize   dq ?
    struct
      FileSizeLow  dd ?
      FileSizeHigh dd ?
    ends
  ends
  FileName     rb 264
ends

struct FileInfoW
  Attributes   dd ?
  Flags        dd ?
  DateCreate   FileDateTime
  DateAccess   FileDateTime
  DateModify   FileDateTime
  union
    FileSize   dq ?
    struct
      FileSizeLow  dd ?
      FileSizeHigh dd ?
    ends
  ends
  FileName     rw 264
ends

virtual at 0
         FileInfo            FileInfoA
         FileInfo fix        FileInfoA
  sizeof.FileInfo fix sizeof.FileInfoA
end virtual

FA_READONLY = 00000001b
FA_HIDDEN   = 00000010b
FA_SYSTEM   = 00000100b
FA_LABEL    = 00001000b
FA_FOLDER   = 00010000b
FA_ARCHIVED = 00100000b
FA_NORMAL   = 01000000b
FA_ANY      = 01111111b
