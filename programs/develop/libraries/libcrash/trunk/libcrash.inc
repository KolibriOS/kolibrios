;    libcrash -- cryptographic hash functions
;
;    Copyright (C) 2012-2014,2016 Ivan Baravy (dunkaist)
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


LIBCRASH_CRC32          = 0
LIBCRASH_MD4            = 1
LIBCRASH_MD5            = 2
LIBCRASH_SHA1           = 3
LIBCRASH_SHA224         = 4
LIBCRASH_SHA256         = 5
LIBCRASH_SHA384         = 6
LIBCRASH_SHA512         = 7
LIBCRASH_SHA3_224       = 8
LIBCRASH_SHA3_256       = 9
LIBCRASH_SHA3_384       = 10
LIBCRASH_SHA3_512       = 11
LIBCRASH_LAST           = 11


struct crash_item
        init    dd ?
        update  dd ?
        final   dd ?
        len_out dd ?
ends

