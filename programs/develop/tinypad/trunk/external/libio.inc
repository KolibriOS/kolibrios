;-----------------------------------------------------------------------------
; Copyright (c) 2006-2007, mike.dld
;-----------------------------------------------------------------------------
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
; DEALINGS IN THE SOFTWARE.
;-----------------------------------------------------------------------------

O_READ	 = 00000001b
O_WRITE  = 00000010b
O_CREATE = 00000100b
O_SHARE  = 00001000b

SEEK_SET = 0
SEEK_CUR = 1
SEEK_END = 2

struct DateTime
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
  DateCreate   DateTime
  DateAccess   DateTime
  DateModify   DateTime
  FileSize     dd ?
  FileSizeHigh dd ?
  FileName     rb 264
ends

struct FileInfoW
  Attributes   dd ?
  Flags        dd ?
  DateCreate   DateTime
  DateAccess   DateTime
  DateModify   DateTime
  FileSize     dd ?
  FileSizeHigh dd ?
  FileName     rw 260
ends

virtual at 0
	 FileInfo	     FileInfoA
	 FileInfo fix	     FileInfoA
  sizeof.FileInfo fix sizeof.FileInfoA
end virtual

FA_READONLY = 00000001b
FA_HIDDEN   = 00000010b
FA_SYSTEM   = 00000100b
FA_LABEL    = 00001000b
FA_FOLDER   = 00010000b
FA_ARCHIVED = 00100000b
