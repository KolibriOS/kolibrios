;;================================================================================================;;
;;//// libio_p.inc //// (c) mike.dld, 2007-2008 //////////////////////////////////////////////////;;
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


F70_READ_F     = 0
F70_READ_D     = 1
F70_CREATE_F   = 2
F70_WRITE_F    = 3
F70_SETSIZE_F  = 4
F70_GETATTR_FD = 5
F70_SETATTR_FD = 6
F70_START_F    = 7
F70_DELETE_FD  = 8
F70_CREATE_D   = 9

F70_ERR_OK		   = 0
F70_ERR_INVALID_DRIVE_SPEC = 1
F70_ERR_FUNC_NOT_SUPPORTED = 2
F70_ERR_UNKNOWN_FS	   = 3
F70_ERR_FILE_NOT_FOUND	   = 5
F70_ERR_END_OF_FILE	   = 6
F70_ERR_INVALID_POINTER    = 7
F70_ERR_NO_DISK_SPACE	   = 8
F70_ERR_FAT_CORRUPTED	   = 9
F70_ERR_ACCESS_DENIED	   = 10
F70_ERR_DEVICE_ERROR	   = 11
F70_ERR_NOT_ENOUGH_MEMORY  = 30
F70_ERR_NOT_EXECUTABLE	   = 31
F70_ERR_TOO_MANY_PROCESSES = 32

struct FindOptions
  Attributes dd ?
  Mask	     dd ?
ends

struct FindFileBlock
  Header    FileInfoHeader
  Info	    FileInfo
  InfoBlock FileInfoBlock
  Options   FindOptions
ends

struct InternalFileInfo
  Mode	   dd ?
  Position dd ?
  FileName rb 260
ends
