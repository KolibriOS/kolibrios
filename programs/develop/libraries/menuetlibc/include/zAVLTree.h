/*
 * zAVLTree.h: Header file for AVLTrees with string keys.
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

#ifndef _ZAVLTREE_H_
#define _ZAVLTREE_H_


typedef struct _zAVLNode {
  const char *key;
  long depth;
  void *item;
  struct _zAVLNode *parent;
  struct _zAVLNode *left;
  struct _zAVLNode *right;
} zAVLNode;


typedef struct {
  zAVLNode *top;
  long count;
  const char *(*getkey)(const void *item);
} zAVLTree;


typedef struct {
  const zAVLTree *avltree;
  const zAVLNode *curnode;
} zAVLCursor;


extern zAVLTree *zAVLAllocTree (const char *(*getkey)(void const *item));
extern void zAVLFreeTree (zAVLTree *avltree, void (freeitem)(void *item));
extern int zAVLInsert (zAVLTree *avltree, void *item);
extern void *zAVLSearch (zAVLTree const *avltree, const char *key);
extern int zAVLDelete (zAVLTree *avltree, const char *key);
extern void *zAVLFirst (zAVLCursor *avlcursor, zAVLTree const *avltree);
extern void *zAVLNext (zAVLCursor *avlcursor);


#endif
