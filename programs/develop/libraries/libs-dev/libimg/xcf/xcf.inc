;;================================================================================================;;
;;//// xcf.inc //// (c) dunkaist, 2011 ///////////////////////////////////////////////////////////;;
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

struct xcf_header
    magic_string    rb  9
    version         rd  1
    reserved        rb  1
    width           rd  1
    height          rd  1
    base_type       rd  1
ends

XCF_BASETYPE_RGB        equ 0
XCF_BASETYPE_GRAY       equ 1
XCF_BASETYPE_INDEXED    equ 2

struct xcf_ext
    visible         rd  1
    layer_mode      rd  1
    offset_x        rd  1
    offset_y        rd  1
    opacity         rd  1
    apply_mask      rd  1
    type            rd  1
    planes          rd  1
ends