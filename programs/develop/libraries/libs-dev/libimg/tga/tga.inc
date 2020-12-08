;;================================================================================================;;
;;//// tga.inc //// (c) Nable, 2007-2008, (c) dunkaist, 2012 /////////////////////////////////////;;
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

struct tga_colormap_spec
    first_entry_index   dw ?
    colormap_length     dw ?
    entry_size          db ?
ends

struct tga_image_spec
    x_origin        dw ?
    y_origin        dw ?
    width           dw ?
    height          dw ?
    depth           db ?
    descriptor      db ?
ends

struct tga_header
    id_length       db ?
    colormap_type   db ?
    image_type      db ?
    colormap_spec   tga_colormap_spec
    image_spec      tga_image_spec
ends

TGA_START_TOP   = 0x20
TGA_START_RIGHT = 0x10
