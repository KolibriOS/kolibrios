/*
byteswap.h - C preprocessor macros for byte swapping.
Copyright (C) 1995, 1996 Michael Riepe <michael@stud.uni-hannover.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* @(#) $Id: byteswap.h,v 1.2 1996/10/22 16:45:54 michael Exp $ */

#ifndef _BYTESWAP_H
#define _BYTESWAP_H

#define lu(from,i,s)		(((long)((unsigned char*)(from))[i])<<(s))
#define li(from,i,s)		(((long)((signed char*)(from))[i])<<(s))

#define __load_u16L(from)	((unsigned long)(lu(from,1,8)|lu(from,0,0)))
#define __load_u16M(from)	((unsigned long)(lu(from,0,8)|lu(from,1,0)))
#define __load_i16L(from)	((long)(li(from,1,8)|lu(from,0,0)))
#define __load_i16M(from)	((long)(li(from,0,8)|lu(from,1,0)))

#define __load_u32L(from)	((unsigned long)(lu(from,3,24)|	\
						 lu(from,2,16)|	\
						 lu(from,1,8)|	\
						 lu(from,0,0)))
#define __load_u32M(from)	((unsigned long)(lu(from,0,24)|	\
						 lu(from,1,16)|	\
						 lu(from,2,8)|	\
						 lu(from,3,0)))
#define __load_i32L(from)	((long)(li(from,3,24)|	\
					lu(from,2,16)|	\
					lu(from,1,8)|	\
					lu(from,0,0)))
#define __load_i32M(from)	((long)(li(from,0,24)|	\
					lu(from,1,16)|	\
					lu(from,2,8)|	\
					lu(from,3,0)))

#define su(to,i,v,s)		(((char*)(to))[i]=((unsigned long)(v)>>(s)))
#define si(to,i,v,s)		(((char*)(to))[i]=((long)(v)>>(s)))

#define __store_u16L(to,v)	(su(to,1,v,8),su(to,0,v,0))
#define __store_u16M(to,v)	(su(to,0,v,8),su(to,1,v,0))
#define __store_i16L(to,v)	(si(to,1,v,8),si(to,0,v,0))
#define __store_i16M(to,v)	(si(to,0,v,8),si(to,1,v,0))

#define __store_u32L(to,v)	(su(to,3,v,24),	\
				 su(to,2,v,16),	\
				 su(to,1,v,8),	\
				 su(to,0,v,0))
#define __store_u32M(to,v)	(su(to,0,v,24),	\
				 su(to,1,v,16),	\
				 su(to,2,v,8),	\
				 su(to,3,v,0))
#define __store_i32L(to,v)	(si(to,3,v,24),	\
				 si(to,2,v,16),	\
				 si(to,1,v,8),	\
				 si(to,0,v,0))
#define __store_i32M(to,v)	(si(to,0,v,24),	\
				 si(to,1,v,16),	\
				 si(to,2,v,8),	\
				 si(to,3,v,0))

#endif /* _BYTESWAP_H */
