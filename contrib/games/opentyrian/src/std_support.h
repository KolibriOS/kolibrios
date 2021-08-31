/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef STD_SUPPORT_H
#define STD_SUPPORT_H

/*!
 * \file std_support.h
 * \brief Standard library support functions
 */

/*!
 * \brief Locate a character in a a string.
 * 
 * \param[in] s the string
 * \param[in] c the character
 * \return the pointer to the first occurrence of \p c in \p s if there is an occurrences;
 *         otherwise the pointer to the terminating null character of \p s
 */
char *ot_strchrnul( const char *s, int c );

#endif // STD_SUPPORT_H
