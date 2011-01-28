#ifndef __SSE2_LIVE_H_INCLUDED_
#define __SSE2_LIVE_H_INCLUDED_

//
// In DLL:
//
// void __stdcall OneGeneration(int w, int h, void *dest, const void *src, int flag);
//

inline int GetDataWidth(int w) {return (w + 63) & ~63;}
inline int GetDataHeight(int h) {return (h + 7) & ~7;}
inline int GetDataNBlock(int w) {return GetDataWidth(w) / 64;}
inline int GetDataPeriod(int h) {return GetDataHeight(h) * 8;}
inline int GetDataSize(int w, int h) {return GetDataNBlock(w) * GetDataPeriod(h);}
const int DataMemAdd = 16;  // using: new char[GetDataSize(w, h) + DataMemAdd];
inline void *GetDataAddress(void *data) {return (void*)(((unsigned int)data + 15) & ~15);}

inline bool GetDataBit(unsigned int w0, unsigned int h0, const void *data0, unsigned int x, unsigned int y)
{			// w0 = GetDataWidth(w); h0 = GetDataHeight(h); data0 = GetDataAddress(data);
	unsigned int nblk = w0 >> 6, sblk = h0 >> 1;
	unsigned int x1 = x % nblk, x2 = x / nblk;
	unsigned int y1 = y % sblk, y2 = y / sblk;
	unsigned int index = ((x1*sblk + y1) << 2) + (y2 << 1) + (x2 >> 5);
	return (((const unsigned int*)data0)[index] >> (x2 & 31)) & 1;
}

inline void Set0DataBit(unsigned int w0, unsigned int h0, void *data0, unsigned int x, unsigned int y)
{			// w0 = GetDataWidth(w); h0 = GetDataHeight(h); data0 = GetDataAddress(data);
	unsigned int nblk = w0 >> 6, sblk = h0 >> 1;
	unsigned int x1 = x % nblk, x2 = x / nblk;
	unsigned int y1 = y % sblk, y2 = y / sblk;
	unsigned int index = ((x1*sblk + y1) << 2) + (y2 << 1) + (x2 >> 5);
	((unsigned int*)data0)[index] &= ~(1U << (x2 & 31));
}

inline void Set1DataBit(unsigned int w0, unsigned int h0, void *data0, unsigned int x, unsigned int y)
{			// w0 = GetDataWidth(w); h0 = GetDataHeight(h); data0 = GetDataAddress(data);
	unsigned int nblk = w0 >> 6, sblk = h0 >> 1;
	unsigned int x1 = x % nblk, x2 = x / nblk;
	unsigned int y1 = y % sblk, y2 = y / sblk;
	unsigned int index = ((x1*sblk + y1) << 2) + (y2 << 1) + (x2 >> 5);
	((unsigned int*)data0)[index] |= 1U << (x2 & 31);
}

inline void SetCDataBit(unsigned int w0, unsigned int h0, void *data0, unsigned int x, unsigned int y)
{			// w0 = GetDataWidth(w); h0 = GetDataHeight(h); data0 = GetDataAddress(data);
	unsigned int nblk = w0 >> 6, sblk = h0 >> 1;
	unsigned int x1 = x % nblk, x2 = x / nblk;
	unsigned int y1 = y % sblk, y2 = y / sblk;
	unsigned int index = ((x1*sblk + y1) << 2) + (y2 << 1) + (x2 >> 5);
	((unsigned int*)data0)[index] ^= 1U << (x2 & 31);
}

inline void SetDataBit(unsigned int w0, unsigned int h0, void *data0, unsigned int x, unsigned int y, bool bit)
{
	if (bit) Set1DataBit(w0, h0, data0, x, y);
	else Set0DataBit(w0, h0, data0, x, y);
}

struct APosPixel
{
	int nblk, sblk;
	void *data;
	unsigned int x1, y1, value, *item;

	APosPixel(int w = 0, int h = 0, void *data = 0, int x = 0, int y = 0)
			: nblk(GetDataWidth(w) >> 6), sblk(GetDataHeight(h) >> 1),
			data(GetDataAddress(data)) {SetTo(x, y);}
	void SetTo(int x, int y)
	{
		unsigned int x2, y2;
		x1 = x % nblk; x2 = x / nblk;
		y1 = y % sblk; y2 = y / sblk;
		item = ((unsigned int*)data) + ((x1*sblk + y1) << 2) + (y2 << 1) + (x2 >> 5);
		value = 1U << (x2 & 31);
	}

	bool GetPixel() const {return *item & value;}
	void Set1Pixel() const {*item |= value;}
	void Set0Pixel() const {*item &= ~value;}
	void SetCPixel() const {*item ^= value;}
	void SetPixel(bool c) const {if (c) Set1Pixel(); else Set0Pixel();}

	void AddX1()
	{
		if (int(++x1) < nblk) {item += sblk << 2; return;}
		x1 = 0; item -= (nblk-1) * sblk << 2;
		if (!(value <<= 1)) {value = 1; item++;}
	}
	void SubX1()
	{
		if (int(--x1) >= 0) {item -= sblk << 2; return;}
		x1 = nblk-1; item += (nblk-1) * sblk << 2;
		if (!(value >>= 1)) {value = 0x80000000; item--;}
	}
	void AddY1()
	{
		if (int(++y1) < sblk) {item += 1 << 2; return;}
		y1 = 0; item -= ((sblk-1) << 2) - 2;
	}
	void SubY1()
	{
		if (int(--y1) >= 0) {item -= 1 << 2; return;}
		y1 = sblk-1; item += ((sblk-1) << 2) - 2;
	}
};

#endif  // __SSE2_LIVE_H_INCLUDED_
