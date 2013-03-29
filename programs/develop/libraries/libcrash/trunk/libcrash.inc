;    libcrash -- cryptographic hash functions
;
;    Copyright (C) 2012-2013 Ivan Baravy (dunkaist)
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.

LIBCRASH_ENDIAN_LITTLE	= 0
LIBCRASH_ENDIAN_BIG	= 1

LIBCRASH_MD4	= 0
LIBCRASH_MD5	= 1
LIBCRASH_SHA1	= 2
LIBCRASH_SHA224	= 3
LIBCRASH_SHA256	= 4
LIBCRASH_SHA384	= 5
LIBCRASH_SHA512	= 6

struct crash_item
	function	dd ?
	init_val	dd ?
	postproc	dd ?
	len_in		dd ?
	len_out		dd ?
	len_blk		dd ?
	len_size	dd ?
	endianness	dd ?
ends
