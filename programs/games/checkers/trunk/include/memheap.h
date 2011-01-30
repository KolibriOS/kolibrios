#ifndef __MEMORY_HEAP_RBTREE_H_INCLUDED_
#define __MEMORY_HEAP_RBTREE_H_INCLUDED_

namespace MemoryHeap
{
	typedef unsigned int TMemItem;

	enum {NumTreeSmall = 8 * sizeof(TMemItem)};

// Memory heap interface.

	struct TFreeSpace
	{
		TMemItem Small[NumTreeSmall], Min, SmallMask;
	};

	struct TMemBlock
	{
		TMemItem *Begin;
	};

	bool BlockValid(const TMemBlock &block);   // Is the given memory block valid?
	void *BlockBegin(const TMemBlock &block);   // Return the beginning address of the block.
	void *BlockEnd(const TMemBlock &block);   // Return the ending address of the block.
	TFreeSpace &BlockFreeSpace(const TMemBlock &block);   // Return the free space of the block.

	void InitFreeSpace(TFreeSpace &fs);   // Initialize the free space.
	TMemBlock NullBlock();   // Return null invalid block.
	TMemBlock CreateBlock(void *begin, void *end, TFreeSpace &fs);
			// Create a memory block with the given begin and end and add free space of it to (fs),
			//_ give (BlockAddSize) bytes of the block for it's data.
			//_ (Program can alloc (end - begin - BlockAddSize) bytes after it,
			//_ that must be not less than (MemMinSize) ).
	TMemBlock CreateBlock(void *begin, void *end);
			// Create a memory block with the given begin and end and new free space for it,
			//_ give (BlockAddSizeFS) bytes of the block for it's data.
			//_ (Program can alloc (end - begin - BlockAddSizeFS) bytes after it,
			//_ that must be not less than (MemMinSize) ).
	void ResizeBlock(TMemBlock block, void *new_end);   // Resize the memory block to the given new end.
	void RemoveBlock(TMemBlock block);   // Remove the given memory block.

	void *BlockEndFor(TMemBlock block, unsigned int size);
			// Return the new end of the block needed for (ResizeBlock) to alloc the given size of memory.
	unsigned int BlockSize(TMemBlock block);   // Return the size of the given block.
	unsigned int MemSize(void *mem);   // Return the size of the allocced memory.

	void *Alloc(TFreeSpace &fs, unsigned int size);
			// Alloc a memory in the given free space, give (MemAddSize) bytes for it's data.
	void *ReAlloc(TFreeSpace &fs, unsigned int size, void *mem);
			// ReAlloc the given memory, it must lie in the block with the given free space.
	void Free(TFreeSpace &fs, void *mem);
			// Free the given memory, it must lie in the block with the given free space.

// Macro definitions.

#define MEMORY_HEAP_ALIGN_DOWN(s)  (MemoryHeap::TMemItem(s) & ~(MemoryHeap::MemAlign - 1))
#define MEMORY_HEAP_ALIGN_UP(s)    ((MemoryHeap::TMemItem(s) + (MemoryHeap::MemAlign - 1)) & ~(MemoryHeap::MemAlign - 1))
#define MEMORY_HEAP_ITEM(s,k)  ( ((MemoryHeap::TMemItem*)(s))[(k)] )
#define MEMORY_HEAP_NEXT(s)    (MEMORY_HEAP_ITEM((s),-1))
#define MEMORY_HEAP_PREV(s)    (MEMORY_HEAP_ITEM((s),-2))
#define MEMORY_HEAP_FREE(s)    (MEMORY_HEAP_ITEM((s),-1) & 1)

// Constants.

	enum {MemAlign = sizeof(TMemItem)};
	enum {MemAddSize = MEMORY_HEAP_ALIGN_UP(2 * sizeof(TMemItem))};
	enum {BlockEndSize = MemAddSize};
	enum {BlockAddSize = MEMORY_HEAP_ALIGN_UP(4 * sizeof(TMemItem)) + BlockEndSize};
	enum {BlockAddSizeFS = BlockAddSize + BlockEndSize + MEMORY_HEAP_ALIGN_UP(sizeof(TFreeSpace))};
	enum {MemMinSize = MEMORY_HEAP_ALIGN_UP(2 * sizeof(TMemItem))};

// Inline functions.

	inline bool BlockValid(const TMemBlock &block) {return block.Begin != 0;}

	inline void *BlockBegin(const TMemBlock &block) {return (void*)block.Begin;}

	inline void *BlockEnd(const TMemBlock &block) {return block.Begin ? (void*)block.Begin[1] : 0;}

	inline TFreeSpace &BlockFreeSpace(const TMemBlock &block) {return *(TFreeSpace*)block.Begin[0];}

	inline TMemBlock NullBlock() {TMemBlock block; block.Begin = 0; return block;}

	inline void *BlockEndFor(TMemBlock block, unsigned int size)
	{
		TMemItem last = (TMemItem)block.Begin[1];
		TMemItem prevlast = MEMORY_HEAP_PREV(last);
		return (void*)( (MEMORY_HEAP_FREE(prevlast) ? prevlast : last) + MemAddSize +
						((size <= MemMinSize) ? MemMinSize : MEMORY_HEAP_ALIGN_UP(size)) );
	}

	inline unsigned int BlockSize(TMemBlock block)
	{
		if (!block.Begin) return 0;
		return (unsigned int)(block.Begin[1] - (TMemItem)block.Begin);
	}

	inline unsigned int MemSize(void *mem)
	{
		if (!mem) return 0;
		TMemItem c = (TMemItem)mem;
		return MEMORY_HEAP_NEXT(c) - c - MemAddSize;
	}

// Free space item functions.

	TMemItem _FirstNotZeroBit(TMemItem i)
	{
		TMemItem r = 0;
		while ((i >>= 1) != 0) r++;
		return r;
	}

	void _RBTreeRotate(TMemItem parent, TMemItem item, int side)
	{
		TMemItem temp = MEMORY_HEAP_ITEM(parent,0);
		MEMORY_HEAP_ITEM(item,0) = temp;
		if (temp)
		{
			if (MEMORY_HEAP_ITEM(temp,2) == parent)
			{
				MEMORY_HEAP_ITEM(temp,2) = item;
			}
			else MEMORY_HEAP_ITEM(temp,3) = item;
		}
		temp = MEMORY_HEAP_ITEM(item,side^1);
		if (temp) MEMORY_HEAP_ITEM(temp,0) = parent;
		MEMORY_HEAP_ITEM(parent,side) = temp;
		MEMORY_HEAP_ITEM(parent,0) = item;
		MEMORY_HEAP_ITEM(item,side^1) = parent;
		temp = MEMORY_HEAP_ITEM(parent,1);
		MEMORY_HEAP_ITEM(parent,1) = MEMORY_HEAP_ITEM(item,1);
		MEMORY_HEAP_ITEM(item,1) = temp;
	}

	void InitFreeSpace(TFreeSpace &fs)
	{
		TMemItem i;
		for (i = 0; i <= NumTreeSmall; i++) fs.Small[i] = 0;
		fs.Min = 0; fs.SmallMask = 0;
	}

	void _FreeAdd(TFreeSpace &fs, TMemItem item)
	{
		TMemItem size = MEMORY_HEAP_NEXT(item) - item;
		if (size < MemAddSize + MemMinSize + MemAlign * NumTreeSmall)
		{
			TMemItem s = (size - (MemAddSize + MemMinSize)) / MemAlign;
			TMemItem &addto = fs.Small[s];
			MEMORY_HEAP_ITEM(item,1) = (TMemItem)(&addto);
			MEMORY_HEAP_ITEM(item,0) = (TMemItem)addto;
			if (addto) MEMORY_HEAP_ITEM(addto,1) = item;
			addto = item;
			fs.SmallMask |= TMemItem(1) << s;
			return;
		}
		TMemItem addto = fs.Min, parent, temp;
		MEMORY_HEAP_ITEM(item,2) = 0;
		MEMORY_HEAP_ITEM(item,3) = 0;
		if (!addto)
		{
			MEMORY_HEAP_ITEM(item,0) = 0;
			MEMORY_HEAP_ITEM(item,1) = 1;
			fs.Min = item;
			return;
		}
		MEMORY_HEAP_ITEM(item,1) = 0;
		TMemItem side = 2;
		if (MEMORY_HEAP_NEXT(addto) - addto >= size) fs.Min = item;
		else
		{
			for (;;)
			{
				parent = MEMORY_HEAP_ITEM(addto,0);
				if (!parent) break;
				if (MEMORY_HEAP_NEXT(parent) - parent < size) addto = parent;
				else break;
			}
			for (;;)
			{
				if (MEMORY_HEAP_NEXT(addto) - addto < size)
				{
					temp = MEMORY_HEAP_ITEM(addto,3);
					if (!temp) {side = 3; break;}
					addto = temp;
				}
				else
				{
					temp = MEMORY_HEAP_ITEM(addto,2);
					if (!temp) break;
					addto = temp;
				}
			}
		}
		MEMORY_HEAP_ITEM(item,0) = addto;
		MEMORY_HEAP_ITEM(addto,side) = item;
		for (;;)
		{
			if (MEMORY_HEAP_ITEM(addto,1) != 0) return;
			parent = MEMORY_HEAP_ITEM(addto,0);
			temp = MEMORY_HEAP_ITEM(parent,2);
			if (temp == addto)
			{
				temp = MEMORY_HEAP_ITEM(parent,3);
				side = 2;
			}
			else side = 3;
			if (!temp || MEMORY_HEAP_ITEM(temp,1) != 0) break;
			MEMORY_HEAP_ITEM(addto,1) = 1;
			MEMORY_HEAP_ITEM(temp,1) = 1;
			item = parent;
			addto = MEMORY_HEAP_ITEM(item,0);
			if (!addto) return;
			MEMORY_HEAP_ITEM(item,1) = 0;
		}
		if (MEMORY_HEAP_ITEM(addto,side) != item)
		{
			temp = MEMORY_HEAP_ITEM(item,side);
			if (temp) MEMORY_HEAP_ITEM(temp,0) = addto;
			MEMORY_HEAP_ITEM(addto,side^1) = temp;
			MEMORY_HEAP_ITEM(addto,0) = item;
			MEMORY_HEAP_ITEM(item,side) = addto;
			MEMORY_HEAP_ITEM(item,0) = parent;
			MEMORY_HEAP_ITEM(parent,side) = item;
		}
		else item = addto;
		_RBTreeRotate(parent, item, side);
	}

	void _FreeDel(TFreeSpace &fs, TMemItem item)
	{
		TMemItem size = MEMORY_HEAP_NEXT(item) - item;
		if (size < MemAddSize + MemMinSize + MemAlign * NumTreeSmall)
		{
			TMemItem prev = MEMORY_HEAP_ITEM(item,1);
			TMemItem next = MEMORY_HEAP_ITEM(item,0);
			MEMORY_HEAP_ITEM(prev,0) = next;
			if (next) MEMORY_HEAP_ITEM(next,1) = prev;
			else
			{
				TMemItem s = (size - (MemAddSize + MemMinSize)) / MemAlign;
				if (!fs.Small[s]) fs.SmallMask &= ~(TMemItem(1) << s);
			}
			return;
		}
		TMemItem parent, temp, second, add;
		TMemItem side = 2;
		temp = MEMORY_HEAP_ITEM(item,3);
		if (temp)
		{
			for (;;)
			{
				second = temp;
				temp = MEMORY_HEAP_ITEM(temp,2);
				if (!temp) break;
			}
			if (fs.Min == item) fs.Min = second;
			add = MEMORY_HEAP_ITEM(second,3);
			parent = MEMORY_HEAP_ITEM(second,0);
			if (parent == item) {parent = second; side = 3;}
			else
			{
				temp = MEMORY_HEAP_ITEM(item,3);
				MEMORY_HEAP_ITEM(second,3) = temp;
				MEMORY_HEAP_ITEM(temp,0) = second;
			}
			temp = MEMORY_HEAP_ITEM(item,2);
			MEMORY_HEAP_ITEM(second,2) = temp;
			if (temp) MEMORY_HEAP_ITEM(temp,0) = second;
			temp = MEMORY_HEAP_ITEM(item,0);
			MEMORY_HEAP_ITEM(second,0) = temp;
			if (temp)
			{
				if (MEMORY_HEAP_ITEM(temp,2) == item)
				{
					MEMORY_HEAP_ITEM(temp,2) = second;
				}
				else MEMORY_HEAP_ITEM(temp,3) = second;
			}
			MEMORY_HEAP_ITEM(parent,side) = add;
			if (add) MEMORY_HEAP_ITEM(add,0) = parent;
			bool color = MEMORY_HEAP_ITEM(second,1);
			MEMORY_HEAP_ITEM(second,1) = MEMORY_HEAP_ITEM(item,1);
			if (!color) return;
		}
		else
		{
			if (fs.Min == item) fs.Min = MEMORY_HEAP_ITEM(item,0);
			add = MEMORY_HEAP_ITEM(item,2);
			parent = MEMORY_HEAP_ITEM(item,0);
			if (add) MEMORY_HEAP_ITEM(add,0) = parent;
			if (parent)
			{
				if (MEMORY_HEAP_ITEM(parent,2) == item)
				{
					MEMORY_HEAP_ITEM(parent,2) = add;
				}
				else
				{
					MEMORY_HEAP_ITEM(parent,3) = add;
					side = 3;
				}
			}
			else
			{
				if (add) MEMORY_HEAP_ITEM(add,1) = 1; 
				return;
			}
			if (!MEMORY_HEAP_ITEM(item,1)) return;
		}
		if (add && !MEMORY_HEAP_ITEM(add,1))
		{
			MEMORY_HEAP_ITEM(add,1) = 1;
			return;
		}
		for (;;)
		{
			second = MEMORY_HEAP_ITEM(parent,side^1);
			if (!MEMORY_HEAP_ITEM(second,1))
			{
				_RBTreeRotate(parent, second, side^1);
				second = MEMORY_HEAP_ITEM(parent,side^1);
			}
			temp = MEMORY_HEAP_ITEM(second,side^1);
			if (temp && !MEMORY_HEAP_ITEM(temp,1))
			{
				MEMORY_HEAP_ITEM(temp,1) = 1;
				break;
			}
			temp = MEMORY_HEAP_ITEM(second,side);
			if (temp && !MEMORY_HEAP_ITEM(temp,1))
			{
				_RBTreeRotate(second, temp, side);
				MEMORY_HEAP_ITEM(second,1) = 1;
				second = temp;
				break;
			}
			MEMORY_HEAP_ITEM(second,1) = 0;
			if (!MEMORY_HEAP_ITEM(parent,1))
			{
				MEMORY_HEAP_ITEM(parent,1) = 1;
				return;
			}
			second = parent;
			parent = MEMORY_HEAP_ITEM(second,0);
			if (!parent) return;
			if (MEMORY_HEAP_ITEM(parent,2) == second) side = 2;
			else side = 3;
		}
		_RBTreeRotate(parent, second, side^1);
	}

	TMemItem _FreeFindAfter(TMemItem item, TMemItem size)
	{
		if (!item) return 0;
		TMemItem paritem, s;
		if (MEMORY_HEAP_NEXT(item) - item >= size) return item;
		for (;;)
		{
			paritem = MEMORY_HEAP_ITEM(item,0);
			if (!paritem) break;
			s = MEMORY_HEAP_NEXT(paritem) - paritem;
			if (s == size) return paritem;
			if (s < size) item = paritem;
			else break;
		}
		MEMORY_HEAP_ITEM(item,3);
		for (;;)
		{
			if (!item) return paritem;
			s = MEMORY_HEAP_NEXT(item) - item;
			if (s == size) return item;
			if (s < size) item = MEMORY_HEAP_ITEM(item,3);
			else
			{
				paritem = item;
				item = MEMORY_HEAP_ITEM(item,2);
			}
		}
	}

	TMemItem _FreeFind(TFreeSpace &fs, TMemItem size)
	{
		TMemItem item, nextitem, s;
		if (size < MemAddSize + MemMinSize + MemAlign * NumTreeSmall)
		{
			TMemItem m, t;
			s = (size - (MemAddSize + MemMinSize)) / MemAlign;
			item = fs.Small[s];
			if (item) return item;
			if (size < MemAlign * NumTreeSmall)
			{
				t = size / MemAlign;
				m = fs.SmallMask >> t;
				if (m) return fs.Small[t + _FirstNotZeroBit(m)];
				else if (fs.Min) return fs.Min;
			}
			else
			{
				item = _FreeFindAfter(fs.Min, size + 1 + MemAddSize + MemMinSize);
				if (item) return item;
			}
			m = fs.SmallMask >> s;
			if (m) return fs.Small[s + _FirstNotZeroBit(m)];
			else return fs.Min;
		}
		item = _FreeFindAfter(fs.Min, ++size);
		if (!item) return 0;
		s = MEMORY_HEAP_NEXT(item) - item;
		if (s == size) return item;
		size += MemAddSize + MemMinSize;
		if (s >= size) return item;
		nextitem = _FreeFindAfter(item, size);
		return nextitem ? nextitem : item;
	}

// Block functions.

	inline void _CreateBlockEnd(TMemBlock &block, TFreeSpace &fs, TMemItem c, TMemItem e)
	{
		block.Begin[0] = (TMemItem)(&fs);
		if (e - c < TMemItem(MemAddSize + MemMinSize))
		{
			MEMORY_HEAP_NEXT(c) = 0;
			block.Begin[1] = c;
		}
		else
		{
			MEMORY_HEAP_NEXT(c) = e + 1;
			_FreeAdd(fs, c);
			MEMORY_HEAP_PREV(e) = c;
			MEMORY_HEAP_NEXT(e) = 0;
			block.Begin[1] = e;
		}
	}

	TMemBlock CreateBlock(void *begin, void *end, TFreeSpace &fs)
	{
		TMemBlock block = {0};
		TMemItem b = MEMORY_HEAP_ALIGN_UP(begin);
		TMemItem e = MEMORY_HEAP_ALIGN_DOWN(end);
		if (e <= b || e - b < TMemItem(BlockAddSize - MemAddSize)) return block;
		block.Begin = (TMemItem*)b;
		b += MEMORY_HEAP_ALIGN_UP(4 * sizeof(TMemItem));
		MEMORY_HEAP_PREV(b) = 0;
		_CreateBlockEnd(block, fs, b, e);
		return block;
	}

	TMemBlock CreateBlock(void *begin, void *end)
	{
		TMemBlock block = {0};
		TMemItem b = MEMORY_HEAP_ALIGN_UP(begin);
		TMemItem e = MEMORY_HEAP_ALIGN_DOWN(end);
		if (e <= b || e - b < TMemItem(BlockAddSizeFS - MemAddSize)) return block;
		block.Begin = (TMemItem*)b;
		b += MEMORY_HEAP_ALIGN_UP(4 * sizeof(TMemItem));
		TMemItem c = b + MemAddSize + MEMORY_HEAP_ALIGN_UP(sizeof(TFreeSpace));
		MEMORY_HEAP_PREV(b) = 0;
		MEMORY_HEAP_NEXT(b) = c;
		MEMORY_HEAP_PREV(c) = b;
		InitFreeSpace(*(TFreeSpace*)b);
		_CreateBlockEnd(block, *(TFreeSpace*)b, c, e);
		return block;
	}

	void ResizeBlock(TMemBlock block, void *new_end)
	{
		if (!BlockValid(block)) return;
		TMemItem e = MEMORY_HEAP_ALIGN_DOWN(new_end);
		TMemItem c = block.Begin[1];
		TFreeSpace &fs = *(TFreeSpace*)block.Begin[0];
		do
		{
			if (c == e) return;
			else if (c > e)
			{
				while ((c = MEMORY_HEAP_PREV(c)) > e)
				{
					if (MEMORY_HEAP_FREE(c)) _FreeDel(fs, c);
				}
				if (!c) {block.Begin = 0; return;}
				if (MEMORY_HEAP_FREE(c))
				{
					_FreeDel(fs, c);
					if (e - c < TMemItem(MemAddSize + MemMinSize)) e = c;
					else
					{
						MEMORY_HEAP_NEXT(c) = e + 1;
						_FreeAdd(*(TFreeSpace*)block.Begin[0], c);
						break;
					}
				}
				else if (e - c >= TMemItem(MemAddSize + MemMinSize))
				{
					MEMORY_HEAP_NEXT(c) = e; break;
				}
				MEMORY_HEAP_NEXT(c) = 0;
				block.Begin[1] = c;
				if (c == e) return;
			}
			TMemItem pc = MEMORY_HEAP_PREV(c);
			if (pc && MEMORY_HEAP_FREE(pc)) _FreeDel(fs, c = pc);
			else if (e - c < TMemItem(MemAddSize + MemMinSize)) return;
			MEMORY_HEAP_NEXT(c) = e + 1;
			_FreeAdd(fs, c);
		} while(false);
		MEMORY_HEAP_PREV(e) = c;
		MEMORY_HEAP_NEXT(e) = 0;
		block.Begin[1] = e;
	}

	void RemoveBlock(TMemBlock block)
	{
		if (!BlockValid(block)) return;
		TMemItem e = block.Begin[1];
		TFreeSpace &fs = *(TFreeSpace*)block.Begin[0];
		while ((e = MEMORY_HEAP_PREV(e)) != 0)
		{
			if (MEMORY_HEAP_FREE(e)) _FreeDel(fs, e);
		}
		block.Begin = 0;
	}

// Free space functions.

	void _CopyMemItemArray(TMemItem dest, TMemItem src, TMemItem end)
	{
		TMemItem k = (end - src) / sizeof(TMemItem);
		TMemItem *d = (TMemItem*)dest;
		TMemItem *s = (TMemItem*)src;
		while (k--) *(d++) = *(s++);
	}

	void *Alloc(TFreeSpace &fs, unsigned int size)
	{
		if (!size) return 0;
		TMemItem s = MEMORY_HEAP_ALIGN_UP(size) + MemAddSize;
		if (s < MemAddSize + MemMinSize) s = MemAddSize + MemMinSize;
		TMemItem c = _FreeFind(fs, s);
		if (!c) return 0;
		_FreeDel(fs, c);
		TMemItem nc = --MEMORY_HEAP_NEXT(c);
		TMemItem mc = c + s;
		if (nc - (MemAddSize + MemMinSize) >= mc)
		{
			MEMORY_HEAP_NEXT(c) = mc;
			MEMORY_HEAP_PREV(mc) = c;
			MEMORY_HEAP_NEXT(mc) = nc + 1;
			MEMORY_HEAP_PREV(nc) = mc;
			_FreeAdd(fs, mc);
		}
		return (void*)c;
	}

	void *ReAlloc(TFreeSpace &fs, void *mem, unsigned int size)
	{
		if (!mem) return Alloc(fs, size);
		if (!size) {Free(fs, mem); return 0;}
		TMemItem s = MEMORY_HEAP_ALIGN_UP(size) + MemAddSize;
		TMemItem c = (TMemItem)mem;
		TMemItem mc = MEMORY_HEAP_NEXT(c);
		TMemItem nc = MEMORY_HEAP_NEXT(mc);
		if (--nc & 1) nc = mc;
		if (s < MemAddSize + MemMinSize) s = MemAddSize + MemMinSize;
		if (nc - c < s)
		{
			mem = Alloc(fs, size);
			if (mem)
			{
				_CopyMemItemArray((TMemItem)mem, c, mc - MemAddSize);
				Free(fs, (void*)c);
				return mem;
			}
			else
			{
				TMemItem pc = MEMORY_HEAP_PREV(c);
				if (pc && MEMORY_HEAP_FREE(pc) && nc - pc >= s)
				{
					_FreeDel(fs, pc);
					_CopyMemItemArray(pc, c, mc - MemAddSize);
					c = pc;
				}
				else return 0;
			}
		}
		if (mc < nc) _FreeDel(fs, mc);
		mc = c + s;
		if (nc - (MemAddSize + MemMinSize) >= mc)
		{
			MEMORY_HEAP_NEXT(c) = mc;
			MEMORY_HEAP_PREV(mc) = c;
			MEMORY_HEAP_NEXT(mc) = nc + 1;
			MEMORY_HEAP_PREV(nc) = mc;
			_FreeAdd(fs, mc);
		}
		else
		{
			MEMORY_HEAP_NEXT(c) = nc;
			MEMORY_HEAP_PREV(nc) = c;
		}
		return (void*)c;
	}

	int free_a = 0;

	void Free(TFreeSpace &fs, void *mem)
	{
		TMemItem c = (TMemItem)mem;
		if (!c) return;
		TMemItem pc = MEMORY_HEAP_PREV(c);
		TMemItem mc = MEMORY_HEAP_NEXT(c);
		TMemItem nc = MEMORY_HEAP_NEXT(mc);
		if (--nc & 1) nc = mc;
		else _FreeDel(fs, mc);
		if (free_a == 1) return;
		if (pc && MEMORY_HEAP_FREE(pc)) _FreeDel(fs, c = pc);
		MEMORY_HEAP_NEXT(c) = nc + 1;
		MEMORY_HEAP_PREV(nc) = c;
		if (free_a == 2) return;
		_FreeAdd(fs, c);
	}
}

#endif  // ndef __MEMORY_HEAP_RBTREE_H_INCLUDED_

