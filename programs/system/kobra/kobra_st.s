/***************************************************************************************************
 *  Copyright (C) Vasiliy Kosenko (vkos), 2009                                                     *
 *  Kobra is free software: you can redistribute it and/or modify it under the terms of the GNU    *
 *  General Public License as published by the Free Software Foundation, either version 3          *
 *  of the License, or (at your option) any later version.                                         *
 *                                                                                                 *
 *  Kobra is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without     *
 *  even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  *
 *  General Public License for more details.                                                       *
 *                                                                                                 *
 *  You should have received a copy of the GNU General Public License along with Kobra.            *
 *  If not, see <http://www.gnu.org/licenses/>.                                                    *
 ***************************************************************************************************/

.global start, entry
.extern main

Start:

.byte 'M', 'E', 'N', 'U', 'E', 'T', '0', '1' 
.long 0x1
.long begin
.long end
.long end+0x4000
.long end+0x4000
.long 0
.long 0

begin:
	call main
