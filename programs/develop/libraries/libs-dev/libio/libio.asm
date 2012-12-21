;;================================================================================================;;
;;//// libio.asm //// (c) mike.dld, 2006-2008 ////////////////////////////////////////////////////;;
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
;;                                                                                                ;;
;; 2008-08-06 (mike.dld)                                                                          ;;
;;   changes:                                                                                     ;;
;;     - split private procs into libio_p.asm, added comments                                     ;;
;; 2007-12-10 (mike.dld)                                                                          ;;
;;   changes:                                                                                     ;;
;;     - almost fully incompatible with previous version since return values were changed.        ;;
;;       now they are more C-like                                                                 ;;
;;   notes:                                                                                       ;;
;;     - `file.err` is not yet available                                                          ;;
;; 2007-09-26 (mike.dld)                                                                          ;;
;;   changes:                                                                                     ;;
;;     - modified `file.size` a bit (according to changes in FileInfo struct)                     ;;
;;     - added `file.find_first`, `file.find_next`, `file.find_close`                             ;;
;;   notes:                                                                                       ;;
;;     - `file.aux.match_wildcard` is exported only for testing purposes, don't                   ;;
;;       use it since it may be removed or renamed in next versions                               ;;
;;                                                                                                ;;
;;================================================================================================;;



format MS COFF

public @EXPORT as 'EXPORTS'

include '../../../../proc32.inc'
include '../../../../macros.inc'
purge section,mov,add,sub

include 'libio.inc'
include 'libio_p.inc'

section '.flat' code readable align 16

include 'libio_p.asm'

;;================================================================================================;;
proc file.find_first _dir, _mask, _attr ;/////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find first file with matching attributes and mask in specified directory                       ;;
;;------------------------------------------------------------------------------------------------;;
;> _dir = directory path to search in <asciiz>                                                    ;;
;> _mask = file mask, with use of wildcards <asciiz>                                              ;;
;> _attr = file attributes mask (combination of FA_* constants) <dword>                           ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / matched file data pointer (acts as find descriptor) <FileInfo*>              ;;
;;================================================================================================;;
        push    ebx edx

        invoke  mem.alloc, sizeof.FindFileBlock
        or      eax, eax
        jz      .exit.error
        mov     edx, eax
        mov     ebx, [_attr]
        mov     [edx + FindFileBlock.Options.Attributes], ebx
        mov     ebx, [_mask]
        mov     [edx + FindFileBlock.Options.Mask], ebx

        lea     ebx, [edx + FindFileBlock.InfoBlock]
        mov     [ebx + FileInfoBlock.Function], F70_READ_D
        mov     [ebx + FileInfoBlock.Count], 1
        lea     eax, [edx + FindFileBlock.Header]
        mov     [ebx + FileInfoBlock.Buffer], eax
        mov     eax, [_dir]
        mov     [ebx + FileInfoBlock.FileName], eax

        stdcall libio._.find_matching_file, edx
        pop     edx ebx
        ret

  .exit.error:
        xor     eax, eax
        pop     edx ebx
        ret
endp

;;================================================================================================;;
proc file.find_next _findd ;//////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find next file matching criteria                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;> _findd = find descriptor (see `file.find_first`) <FileInfo*>                                   ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / matched file data pointer (acts as find descriptor) <FileInfo*>              ;;
;;================================================================================================;;
        mov     eax, [_findd]
        add     eax, -sizeof.FileInfoHeader
        inc     [eax + FindFileBlock.InfoBlock.Position]
        stdcall libio._.find_matching_file, eax
        ret
endp

;;================================================================================================;;
proc file.find_close _findd ;/////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Close find descriptor and free memory                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;> _findd = find descriptor (see `file.find_first`) <FileInfo*>                                   ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = result of memory freeing routine                                                         ;;
;;================================================================================================;;
        mov     eax, [_findd]
        add     eax, -sizeof.FileInfoHeader
        invoke  mem.free, eax
        ret
endp

;;================================================================================================;;
proc file.size _name ;////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Get file size                                                                                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _name = path to file (full or relative) <asciiz>                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< ebx = -1 (error) / file size (in bytes, up to 2G) <dword>                                      ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
locals
  loc_info FileInfoBlock
endl

        lea     ebx, [loc_info]
        invoke  mem.alloc, 40
        push    eax
        mov     [ebx + FileInfoBlock.Function], F70_GETATTR_FD
        mov     [ebx + FileInfoBlock.Buffer], eax
        mov     byte[ebx + FileInfoBlock.FileName - 1], 0
        mov     eax, [_name]
        mov     [ebx + FileInfoBlock.FileName], eax
        mcall   70
        pop     ebx
        push    eax
        mov     eax, ebx
        mov     ebx, [ebx + FileInfo.FileSizeLow]
        invoke  mem.free, eax
        pop     eax
        ret
endp

;;================================================================================================;;
proc file.open _name, _mode ;/////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Open file                                                                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _name = path to file (full or relative) <asciiz>                                               ;;
;> _mode = mode to open file in (combination of O_* constants) <dword>                            ;;
;>   O_BINARY - don't change read/written data in any way (default)                               ;;
;>   O_READ - open file for reading                                                               ;;
;>   O_WRITE - open file for writing                                                              ;;
;>   O_CREATE - create file if it doesn't exist, open otherwise                                   ;;
;>   O_SHARE - allow simultaneous access by using different file descriptors (not implemented)    ;;
;>   O_TEXT - replace newline chars with LF (overrides O_BINARY, not implemented)                 ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / file descriptor <InternalFileInfo*>                                          ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
locals
  loc_info FileInfoBlock
  loc_buf  rb 40
endl

        push    ebx esi edi

        xor     ebx, ebx
        invoke  mem.alloc, sizeof.InternalFileInfo
        or      eax, eax
        jz      .exit_error
        mov     ebx, eax
        push    [_mode]
        pop     [ebx + InternalFileInfo.Mode]
        mov     [ebx + InternalFileInfo.Position], 0
        lea     edi, [ebx + InternalFileInfo.FileName]
        mov     esi, [_name]
        mov     ecx, 260 / 4
        cld
        rep     movsd

  .get_info:
        push    ebx
        mov     [loc_info.Function], F70_GETATTR_FD
        lea     eax, [loc_buf]
        mov     [loc_info.Buffer], eax
        mov     byte[loc_info.FileName - 1], 0
        mov     eax, [_name]
        mov     [loc_info.FileName], eax
        lea     ebx, [loc_info]
        mcall   70
        pop     ebx
        or      eax, eax
        jz      @f
        cmp     eax, 6
        jne     .exit_error.ex
    @@:
        mov     eax, ebx
        pop     edi esi ebx
        ret

  .exit_error.ex:
        test    [_mode], O_CREATE
        jz      .exit_error
        push    ebx
        mov     [loc_info.Function], F70_CREATE_F
        xor     eax, eax
        mov     [loc_info.Position], eax
        mov     [loc_info.Flags], eax
        mov     [loc_info.Count], eax
        lea     ebx, [loc_info]
        mcall   70
        pop     ebx
        or      eax, eax
        jz      .get_info

  .exit_error:
        invoke  mem.free, ebx
        xor     eax, eax
        pop     edi esi ebx
        ret
endp

;;================================================================================================;;
proc file.read _filed, _buf, _buflen ;////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Read data from file                                                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`) <InternalFileInfo*>                                 ;;
;> _buf = buffer to put read data to <byte*>                                                      ;;
;> _buflen = buffer size (number of bytes to be read from file) <dword>                           ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / number of bytes read <dword>                                                ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
locals
  loc_info FileInfoBlock
endl

        push    ebx esi edi

        mov     ebx, [_filed]
        test    [ebx + InternalFileInfo.Mode], O_READ
        jz      .exit_error

        xor     eax, eax
        mov     [loc_info.Function], F70_READ_F
        mov     [loc_info.Flags], eax
        mov     byte[loc_info.FileName - 1], al
        push    [ebx+InternalFileInfo.Position] [_buflen] [_buf]
        pop     [loc_info.Buffer] [loc_info.Count] [loc_info.Position]
        lea     eax, [ebx + InternalFileInfo.FileName]
        mov     [loc_info.FileName], eax
        lea     ebx, [loc_info]
        mcall   70
        or      eax, eax
        jz      @f
        cmp     eax, 6
        jne     .exit_error
    @@:
        mov     eax, ebx
        mov     ebx, [_filed]
        add     [ebx + InternalFileInfo.Position], eax
        pop     edi esi ebx
        ret

  .exit_error:
        or      eax, -1
        pop     edi esi ebx
        ret
endp

;;================================================================================================;;
proc file.write _filed, _buf, _buflen ;///////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Write data to file                                                                             ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`) <InternalFileInfo*>                                 ;;
;> _buf = buffer to get write data from <byte*>                                                   ;;
;> _buflen = buffer size (number of bytes to be written to file) <dword>                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / number of bytes written <dword>                                             ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
locals
  loc_info FileInfoBlock
endl

        push    ebx esi edi

        mov     ebx, [_filed]
        test    [ebx + InternalFileInfo.Mode], O_WRITE
        jz      .exit_error

        stdcall file.eof?, [_filed]
        or      eax, eax
        js      .exit_error
        jz      @f
        stdcall file.truncate, [_filed]
    @@:
        mov     [loc_info.Function], F70_WRITE_F
        xor     eax, eax
        mov     [loc_info.Flags], eax
        mov     byte[loc_info.FileName - 1], al
        push    [ebx + InternalFileInfo.Position] [_buflen] [_buf]
        pop     [loc_info.Buffer] [loc_info.Count] [loc_info.Position]
        lea     eax, [ebx + InternalFileInfo.FileName]
        mov     [loc_info.FileName], eax
        lea     ebx, [loc_info]
        mcall   70
        or      eax, eax
        jnz     .exit_error
    @@:
        mov     eax, ebx
        mov     ebx, [_filed]
        add     [ebx + InternalFileInfo.Position],eax
        pop     edi esi ebx
        ret

  .exit_error:
        or      eax, -1
        pop     edi esi ebx
        ret
endp

;;================================================================================================;;
proc file.seek _filed, _where, _origin ;//////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Set file pointer position                                                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`) <InternalFileInfo*>                                 ;;
;> _where = position in file (in bytes) counted from specified origin <dword>                     ;;
;> _origin = origin from where to set the position (one of SEEK_* constants) <dword>              ;;
;>   SEEK_SET - from beginning of file                                                            ;;
;>   SEEK_CUR - from current pointer position                                                     ;;
;>   SEEK_END - from end of file                                                                  ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0 <dword>                                                                   ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
        push    ebx ecx edx

        mov     ecx, [_filed]
        lea     eax, [ecx + InternalFileInfo.FileName]
        stdcall file.size, eax
        or      eax, eax
        jnz     .exit_error
        mov     edx, [_where]
        cmp     [_origin], SEEK_SET
        jne     .n_set
        mov     [ecx + InternalFileInfo.Position], edx
        jmp     .exit_ok

  .n_set:
        cmp     [_origin], SEEK_CUR
        jne     .n_cur
        add     [ecx + InternalFileInfo.Position], edx
        jmp     .exit_ok

  .n_cur:
        cmp     [_origin], SEEK_END
        jne     .exit_error
        neg     edx
        add     edx, ebx
        mov     [ecx + InternalFileInfo.Position], edx

  .exit_ok:

        cmp     [ecx + InternalFileInfo.Position], 0
        jge     @f
        mov     [ecx + InternalFileInfo.Position], 0
    @@:
;       cmp     ebx, [ecx+InternalFileInfo.Position]
;       jae     @f
;       mov     [ecx + InternalFileInfo.Position], ebx
;   @@:
        xor     eax, eax
        pop     edx ecx ebx
        ret

  .exit_error:
        or      eax, -1
        pop     edx ecx ebx
        ret
endp

;;================================================================================================;;
proc file.eof? _filed ;///////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if file pointer is at the end of file                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`) <InternalFileInfo*>                                 ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true <dword>                                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
        push    ebx ecx

        mov     ecx, [_filed]
        lea     eax, [ecx + InternalFileInfo.FileName]
        stdcall file.size, eax
        or      eax, eax
        jnz     .exit_error

        xor     eax, eax
        cmp     [ecx + InternalFileInfo.Position], ebx
        jb      @f
        inc     eax
    @@: pop     ecx ebx
        ret

  .exit_error:
        or      eax, -1
        pop     ecx ebx
        ret
endp

;;================================================================================================;;
proc file.truncate _filed ;///////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Truncate file size to current file pointer position                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`) <InternalFileInfo*>                                 ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0 <dword>                                                                   ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
locals
  loc_info FileInfoBlock
endl

        push    ebx esi edi

        mov     ebx, [_filed]
        test    [ebx + InternalFileInfo.Mode], O_WRITE
        jz      .exit_error

        mov     [loc_info.Function], F70_SETSIZE_F
        mov     eax, [ebx + InternalFileInfo.Position]
        mov     [loc_info.Position], eax
        xor     eax, eax
        mov     [loc_info.Flags], eax
        mov     [loc_info.Count], eax
        mov     [loc_info.Buffer], eax
        mov     byte[loc_info.FileName - 1], al
        lea     eax, [ebx + InternalFileInfo.FileName]
        mov     [loc_info.FileName], eax
        lea     ebx, [loc_info]
        mcall   70
        cmp     eax, 2
        je      .exit_error
        cmp     eax, 8
        je      .exit_error
    @@: xor     eax, eax
        pop     edi esi ebx
        ret

  .exit_error:
        or      eax, -1
        pop     edi esi ebx
        ret
endp

file.seteof equ file.truncate

;;================================================================================================;;
proc file.tell _filed ;///////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Get current file pointer position                                                              ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`) <InternalFileInfo*>                                 ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / file pointer position <dword>                                               ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
        mov     eax, [_filed]
        mov     eax, [eax + InternalFileInfo.Position]
        ret
endp

;;================================================================================================;;
proc file.close _filed ;//////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Close file                                                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`) <InternalFileInfo*>                                 ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / file pointer position <dword>                                               ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
        cmp eax,32
        jb .exit_error
        mov     eax, [_filed]
        mov     [eax + InternalFileInfo.Mode], 0
        mov     [eax + InternalFileInfo.FileName], 0
        invoke  mem.free, eax
        xor     eax, eax
        jmp @f
        .exit_error:
        or      eax, -1
        @@:
        ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Exported functions section                                                                     ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


align 16
@EXPORT:

export                                        \
        libio._.init    , 'lib_init'        , \
        0x00040004      , 'version'         , \
        file.find_first , 'file_find_first' , \
        file.find_next  , 'file_find_next'  , \
        file.find_close , 'file_find_close' , \
        file.size       , 'file_size'       , \
        file.open       , 'file_open'       , \
        file.read       , 'file_read'       , \
        file.write      , 'file_write'      , \
        file.seek       , 'file_seek'       , \
        file.tell       , 'file_tell'       , \
        file.eof?       , 'file_iseof'      , \
        file.seteof     , 'file_seteof'     , \
        file.truncate   , 'file_truncate'   , \
        file.close      , 'file_close'

