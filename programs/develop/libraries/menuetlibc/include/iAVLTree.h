/*
 * iAVLTree.h: Header file for AVLTrees with long integer keys.
 * Copyright (C) 1998  Michael H. Buselli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * The author of this library can be reached at the following address:
 * Michael H. Buselli
 * 4334 N. Hazel St. #515
 * Chicago, IL  60613-1456
 *
 * Or you can send email to <cosine@tezcat.com>.
 * The official web page for this product is:
 * http://www.tezcat.com/~cosine/pub/AVLTree/
 *
 * This is version 0.1.0 (alpha).
 */

#ifndef _IAVLTREE_H_
#define _IAVLTREE_H_


typedef struct _iAVLNode {
  long key;
  long depth;
  void *item;
  struct _iAVLNode *parent;
  struct _iAVLNode *left;
  struct _iAVLNode *right;
} iAVLNode;


typedef struct {
  iAVLNode *top;
  long count;
  long (*getkey)(const void *item);
} iAVLTree;


typedef struct {
  const iAVLTree *avltree;
  const iAVLNode *curnode;
} iAVLCursor;


extern iAVLTree *iAVLAllocTree (long (*getkey)(void const *item));
extern void iAVLFreeTree (iAVLTree *avltree, void (freeitem)(void *item));
extern int iAVLInsert (iAVLTree *avltree, void *item);
extern void *iAVLSearch (iAVLTree const *avltree, long key);
extern int iAVLDelete (iAVLTree *avltree, long key);
extern void *iAVLFirst (iAVLCursor *avlcursor, iAVLTree const *avltree);
extern void *iAVLNext (iAVLCursor *avlcursor);


#endif
