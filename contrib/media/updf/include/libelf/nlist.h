/*
nlist.h - public header file for nlist(3).
Copyright (C) 1995 - 1998 Michael Riepe <michael@stud.uni-hannover.de>

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

/* @(#) $Id: nlist.h,v 1.3 1998/06/01 19:47:24 michael Exp $ */

#ifndef _NLIST_H
#define _NLIST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct nlist {
    char*		n_name;
    long		n_value;
    short		n_scnum;
    unsigned short	n_type;
    char		n_sclass;
    char		n_numaux;
};

#if __STDC__ || defined(__cplusplus)
extern int nlist(const char *__filename, struct nlist *__nl);
#else /* __STDC__ || defined(__cplusplus) */
extern int nlist();
#endif /* __STDC__ || defined(__cplusplus) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NLIST_H */
