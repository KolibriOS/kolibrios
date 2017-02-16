#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

typedef unsigned int color_t;

void px_rect_alu(void *dst_addr, int pitch, int w, int h, color_t src_color)
{
	while (h--)
	{
		char *tmp = dst_addr;
		dst_addr = tmp + pitch;
        __asm__ __volatile__(
          "cld; rep stosl\n\t"
          :: "D"(tmp), "a"(src_color), "c"(w)
          : "flags");
	};
};

void px_rect_mmx(void *dst_addr, int pitch, int w, int h, color_t src_color)
{
	register __m64 color;

	color = _mm_cvtsi32_si64(src_color);
	color = _mm_unpacklo_pi32(color, color);

	while (h--)
	{
		char *tmp = dst_addr;
		char *end = tmp + w * sizeof(color_t);
		dst_addr = tmp + pitch;

		int t = (int)tmp;
		if (t & 4)
		{
			*(color_t*)tmp = src_color;
			tmp += 4;
		};

		while (tmp + 64 <= end)
		{
			__m64 *_tmp = (__m64*)tmp;
			_tmp[0] = color;
			_tmp[1] = color;
			_tmp[2] = color;
			_tmp[3] = color;
			_tmp[4] = color;
			_tmp[5] = color;
			_tmp[6] = color;
			_tmp[7] = color;
			tmp += 64;
		};
		if (tmp + 32 <= end)
		{
			__m64 *_tmp = (__m64*)tmp;
			_tmp[0] = color;
			_tmp[1] = color;
			_tmp[2] = color;
			_tmp[3] = color;
			tmp += 32;
		};
		if (tmp + 16 <= end)
		{
			__m64 *_tmp = (__m64*)tmp;
			_tmp[0] = color;
			_tmp[1] = color;
			tmp += 16;
		};
		if (tmp + 8 <= end)
		{
			__m64 *_tmp = (__m64*)tmp;
			_tmp[0] = color;
			tmp += 8;
		};
		if (tmp < end)
			*(color_t*)tmp = src_color;
	};
	_mm_empty();
};

void px_rect_xmm(void *dst_addr, int pitch, int w, int h, color_t dst_color)
{
	__m128i color;

	color = _mm_set_epi32(dst_color, dst_color, dst_color, dst_color);

	while (h--)
	{
		char *tmp = dst_addr;
		char *end = tmp + w * sizeof(color_t);
		dst_addr = tmp + pitch;

		if ((int)tmp & 4)
		{
			*(color_t*)tmp = dst_color;
			tmp += 4;
		};
		if ((int)tmp & 8)
		{
			__m128i *_tmp = (__m128i*)tmp;
			_mm_storel_epi64(_tmp, color);
			tmp += 8;
		};

		while (tmp + 128 <= end)
		{
			__m128i *_tmp = (__m128i*)tmp;
			_mm_store_si128(&_tmp[0], color);
			_mm_store_si128(&_tmp[1], color);
			_mm_store_si128(&_tmp[2], color);
			_mm_store_si128(&_tmp[3], color);
			_mm_store_si128(&_tmp[4], color);
			_mm_store_si128(&_tmp[5], color);
			_mm_store_si128(&_tmp[6], color);
			_mm_store_si128(&_tmp[7], color);
			tmp += 128;
		};
		if (tmp + 64 <= end)
		{
			__m128i *_tmp = (__m128i*)tmp;
			_mm_store_si128(&_tmp[0], color);
			_mm_store_si128(&_tmp[1], color);
			_mm_store_si128(&_tmp[2], color);
			_mm_store_si128(&_tmp[3], color);
			tmp += 64;
		};
		if (tmp + 32 <= end)
		{
			__m128i *_tmp = (__m128i*)tmp;
			_mm_store_si128(&_tmp[0], color);
			_mm_store_si128(&_tmp[1], color);
			tmp += 32;
		};
		if (tmp + 16 <= end)
		{
			__m128i *_tmp = (__m128i*)tmp;
			_mm_store_si128(&_tmp[0], color);
			tmp += 16;
		};
		if (tmp + 8 <= end)
		{
			__m128i *_tmp = (__m128i*)tmp;
			_mm_storel_epi64(_tmp, color);
			tmp += 8;
		};
		if (tmp < end)
			*(color_t*)tmp = dst_color;
	};
}

void px_glyph_alu(void *dst_addr, int dst_pitch,const void *src_addr, int src_pitch,
	              int width, int height, color_t src_color)
{
	while (height-- > 0)
	{
		int w = width;
		const unsigned char *src = src_addr;
        color_t *dst = dst_addr;

        dst_addr = (char*)dst + dst_pitch;
		src_addr = src + src_pitch;

		while (w-- > 0)
		{
			unsigned char a = *src++;
			color_t dst_color = *(color_t*)dst;
			unsigned int rb = dst_color & 0xff00ff;
			unsigned int g = dst_color & 0x00ff00;

			rb += ((src_color & 0xff00ff) - rb) * a >> 8;
			g += ((src_color & 0x00ff00) - g) * a >> 8;

            *dst++ = (src_color & 0xFF000000) | (rb & 0xff00ff) | (g & 0xff00);
		};
	}
}

__m64 m_4x0080 = { 0x00800080, 0x00800080 };
__m64 m_4x0101 = { 0x01010101, 0x01010101 };
__m64 m_4x00FF = { 0x00FF00FF, 0x00FF00FF };
__m64 m_zero   = { 0 };

void px_glyph_sse(void *dst_addr, int dst_pitch, const void *src_addr, int src_pitch,
	int width, int height, color_t src_color)
{
    static __m64 m_4x0080 = { 0x00800080, 0x00800080 };
    static __m64 m_4x0101 = { 0x01010101, 0x01010101 };
    static __m64 m_4x00FF = { 0x00FF00FF, 0x00FF00FF };
    static __m64 m_zero   = { 0 };

    __m64 color;

	color = _mm_cvtsi32_si64(src_color);
	color = _m_punpcklbw(color, m_zero);
	while (height-- > 0)
	{
		int w = width;
		const unsigned char *tmpsrc = src_addr;
        color_t *tmpdst = dst_addr;

        dst_addr = (char*)tmpdst + dst_pitch;
		src_addr = tmpsrc + src_pitch;

		while (w-- > 0)
		{
			__m64 m_alpha, m_1_minus_alpha;
			__m64 src_alpha, dst_color;

			unsigned int alpha = *tmpsrc++;
			m_alpha = _mm_cvtsi32_si64((alpha << 16) | alpha);
			dst_color = _mm_cvtsi32_si64(*(int*)tmpdst);

			m_alpha = _mm_unpacklo_pi32(m_alpha, m_alpha);
			m_1_minus_alpha = _m_psubb(m_4x00FF, m_alpha);
			dst_color = _m_punpcklbw(dst_color, m_zero);
			src_alpha = _m_pmullw(color, m_alpha);
			dst_color = _m_pmullw(dst_color, m_1_minus_alpha);
			dst_color = _m_paddw(src_alpha, dst_color);
			dst_color = _m_paddw(dst_color, m_4x0080);
			dst_color = _mm_mulhi_pu16(dst_color, m_4x0101);
			dst_color = _mm_packs_pu16(dst_color, dst_color);
            *tmpdst++ = _mm_cvtsi64_si32(dst_color);
		};
	}
	_mm_empty();
};
