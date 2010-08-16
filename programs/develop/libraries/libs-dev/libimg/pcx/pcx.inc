;;================================================================================================;;
;;//// pcx.inc //// (c) dunkaist, 2010 ///////////////////////////////////////////////////////////;;
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

struct pcx_header
    magic_number    rb  1
    version         rb  1
    encoding        rb  1
    bpp             rb  1
    xmin            rw  1
    ymin            rw  1
    xmax            rw  1
    ymax            rw  1
    hres            rw  1
    vres            rw  1
    colormap        rb  48
    reserved        rb  1
    nplanes         rb  1
    bpl             rw  1
    palette_info    rw  1
    filler          rb  58
ends