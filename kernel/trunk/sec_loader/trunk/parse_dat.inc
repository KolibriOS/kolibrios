; Copyright (c) 2009, <Lrz>
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;       * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;       * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;       * Neither the name of the <organization> nor the
;       names of its contributors may be used to endorse or promote products
;       derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Alexey Teplov aka <Lrz> ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************

;Тут представленны теги, для сравнения
parse_loader      db '[loader]'
parse_loader_e:
parse_l_timeout   db 'timeout'
parse_l_timeout_e:
parse_l_default	  db 'default'
parse_l_default_e:
parse_name	  db 'ame'
parse_name_e:
parse_descript	  db 'descript'
parse_descript_e:

parse_LoaderModule db 'LoaderModule'
parse_LoaderModule_e:
parse_RamdiskSize db 'RamdiskSize'
parse_RamdiskSize_e:  
parse_RamdiskFS   db 'RamdiskFS'
parse_RamdiskFS_e:
parse_RamdiskSector	db 'RamdiskSector'
parse_RamdiskSector_e:
parse_RamdiskCluster	db 'RamdiskCluster'
parse_RamdiskCluster_e:
parse_RFS_FAT	  db 'FAT'
parse_RFS_FAT_e:
parse_RFS_KRFS	  db 'KRFS'
parse_RFS_KRFS_e:
parse_Loader_Image	db 'LoaderImage'
parse_Loader_Image_e:
parse_RamdiskFile	db 'RamdiskFile'
parse_RamdiskFile_e:
