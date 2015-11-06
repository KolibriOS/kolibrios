#ifndef INCLUDE_FONT_H
#define INCLUDE_FONT_H

#ifndef INCLUDE_MATH_H
#include "../lib/math.h"
#endif

#ifndef INCLUDE_IO_H
#include "../lib/obj/fs.h"
#endif

:struct __OFFSET_FONT
{
	signed x,y;
};
:struct __SIZE
{
	dword width,height;
	__OFFSET_FONT offset;
	float offset_i,w_italic;
	byte text;
	byte TMP_WEIGHT;
};
:struct FONT
{
	__SIZE size;
	byte r,g,b,weight,italic, smooth;
	byte width,height;
	byte use_smooth;
	int left,top;
	byte encoding;
	dword color;
	dword file_size;
	dword buffer;
	dword buffer_size;
	word block;
	dword data;
	dword begin;
	byte load(...);
	byte symbol(word x,y;byte s;dword c);
	byte symbol_size(byte s);
	dword prepare(word x,y;dword text1);
	void prepare_buf(dword x,y,w,h, text1);
	void show();
	byte textcenter(word x,y,w,h;dword txt);
	dword getsize(dword text1);
	byte changeSIZE();
	void PixelRGB(dword x,y);
	//dword GetPixel(word x,y);
	byte no_bg_copy;
	dword bg_color;
};
FONT font = 0;
/*
:dword FONT::GetPixel(word x,y)
{
	dword tmp = y*size.width*3;
	tmp += x*3 + buffer;
	r = DSBYTE[tmp]; tmp++;
	g = DSBYTE[tmp]; tmp++;
	b = DSBYTE[tmp];
}*/
:void FONT::PixelRGB(dword x,y)
{
	dword offs = y*size.width+x*3 + buffer;
	DSBYTE[offs] = r; offs++;
	DSBYTE[offs] = g; offs++;
	DSBYTE[offs] = b;
}
:byte FONT::changeSIZE()
{
	dword TMP_DATA;
	dword ofs;
	IF(size.text<9) size.text = 8;
	TMP_DATA = data = begin;
	TMP_DATA +=size.text-8*4;
	ofs = DSDWORD[TMP_DATA];
	IF(ofs==-1)return false;
	data += ofs + 156;
	TMP_DATA = data;
	file_size = DSDWORD[TMP_DATA];
	TMP_DATA = data + file_size;
	height = DSBYTE[TMP_DATA - 1];
	width =  DSBYTE[TMP_DATA - 2];
	block = math.ceil(height*width/32);
	return true;
}
:byte FONT::textcenter(word x,y,w,h;dword txt)
{
	getsize(txt);
	EDX = w/2;
	ECX = size.width/2;
	EDX -= ECX;
	x += EDX;
	prepare(x,y,txt);
}
:dword FONT::getsize(dword text1)
{
	size.height = size.width = 0;
	size.offset.x = size.offset.y = -1;
	IF(size.text)IF(!changeSIZE())return 0;
	WHILE(DSBYTE[text1])
	{
		symbol_size(DSBYTE[text1]);
		text1++;
	}
	$neg size.offset.y
	$neg size.offset.x
	size.height += size.offset.y; size.height++;
	size.width += size.offset.x; size.width++;
	IF(italic)
	{
		size.w_italic = size.height/3;
		size.offset_i = size.w_italic/size.height;
		size.width += size.w_italic;
		size.w_italic = -size.w_italic;
	}
	return size.width;
}
:byte FONT::symbol_size(byte s)
{
        dword xi,yi;
        dword tmp,_;
        dword iii;
        byte rw=0;
		byte X;
		size.TMP_WEIGHT = math.ceil(size.text/17);
        IF(s==32)
		{
			size.width += width/4;
			IF(weight) size.width+=size.TMP_WEIGHT;
			return;
		}
		IF(s==9)
		{
			size.width += width;
			IF(weight) size.width+=size.TMP_WEIGHT;
			return;
		}
		IF(!encoding){
			IF(s>=128)&&(s<=175)s+=64;
			ELSE IF(s>=224)&&(s<=239)s+=16;
			ELSE IF(s==241)s=184; //yo
			ELSE IF(s==240)s=168; //YO
		}
        iii = 0;
        tmp = 4*block*s + data;
        for(yi=0; yi<height; yi++)
        {
            for(xi=0; xi<width; xi++)
            {
                IF(iii%32) _ >>= 1;
				ELSE
				{
                        tmp += 4;
                        _ = DSDWORD[tmp];
                }
                IF(_&1)
				{
					IF(xi>rw)rw=xi;
					IF(size.height<yi)size.height = yi;
					IF(size.offset.y<0)size.offset.y = yi;
					ELSE IF(yi<size.offset.y)size.offset.y = yi;
					IF(!X) X = xi;
					ELSE IF(X>xi)X = xi;
				}
                iii++;
            }
        }
		size.width += rw;
		IF(weight) size.width+=size.TMP_WEIGHT;
		IF(s=='_') size.width--;
		IF(size.offset.x<0)size.offset.x = X;
}
:dword FONT::prepare(int x,y;dword text1)
{
	signed len=0;
	proc_info Form_SELF_FONTS;
	dword c;
	c = color;
	left = x;
	IF(!text1)return false;
	IF(size.text)IF(!changeSIZE())return false;
	AX = c; r = AL; g = AH; c>>=16; AX = c; b = AL;
	getsize(text1);
	y -= size.offset.y;
	top = y;
	EDX = size.width*size.height*3;
	IF(!buffer_size)
	{
		buffer_size = EDX;
		buffer = malloc(buffer_size);
	}
	ELSE IF(buffer_size<EDX)
	{
		buffer_size = EDX;
		buffer = realloc(buffer,buffer_size);
	}
	IF (no_bg_copy)
	{
		EBX = bg_color;
		EDI = buffer;
		EAX = buffer_size+EDI;
		WHILE (EDI<EAX)
		{
			ESDWORD[EDI] = EBX;
			$add edi,3
		}
	}
	ELSE
	{
		GetProcessInfo(#Form_SELF_FONTS, SelfInfo); 
		CopyScreen(buffer,x+Form_SELF_FONTS.left+5,y+Form_SELF_FONTS.top+GetSkinHeight(),size.width,size.height);
	}
	len = size.offset.x;
	WHILE(DSBYTE[text1])
	{
		IF(DSBYTE[text1]=='_') len--;
		len+=symbol(len,0,DSBYTE[text1]);
		IF(weight)len+=math.ceil(size.text/17);
		text1++;
	}
	IF (use_smooth) SmoothFont(buffer, size.width, size.height);
	return len;
}
:void FONT::show()
{
	_PutImage(left,top,size.width,size.height,buffer);
}
inline fastcall dword b24(EBX) { return DSDWORD[EBX] << 8; }
:void SmoothFont(dword image, w, h)
{
	byte rr,gg,bb;
	dword i,line_w,to, pixel;
	line_w = w * 3;
	to = w*h*3 + image - line_w - 3;
	for(i = image; i < to; i+=3)	{
		IF(i-image%line_w +3 == line_w) continue;
		IF(b24(i)==0x000000) && (b24(i+3)!=0x000000) && (b24(i+line_w)!=0x000000) && (b24(i+3+line_w)==0x000000)
		{ 
			ShadowImage(i+3, 1, 1, 2);
			ShadowImage(i+line_w, 1, 1, 2);
		}
		ELSE IF(b24(i)!=0x000000) && (b24(i+3)==0x000000) && (b24(i+line_w)==0x000000) && (b24(i+3+line_w)!=0x000000)
		{
			ShadowImage(i, 1, 1, 2);
			ShadowImage(i+3+line_w, 1, 1, 2);
		}
	}
}
:byte FONT::symbol(signed x,y;byte s)
{
        dword xi,yi;
        dword iii;
		float ital = -size.w_italic;
		dword ___x;
		//byte _TMP_WEIGHT;
        byte rw=0;
		//_TMP_WEIGHT=2;
        IF(s==32)return width/4;
		IF(s==9)return width;
		IF(!encoding)
		{
			IF(s>=128)&&(s<=175)s+=64;
			ELSE IF(s>=224)&&(s<=239)s+=16;
			ELSE IF(s==241)s=184; //yo
			ELSE IF(s==240)s=168; //YO
		}
        iii = 0;
        EBX = block*s;
		$shl ebx,2
		$add ebx,data
		yi = 0;
        while(yi<height)
        {
			EDI = size.offset.y;
			$add edi,yi
			$add edi,y
            for(xi=0; xi<width; xi++)
            {
				IF(iii%32) $shr ecx,1
				ELSE
				{
						$add ebx,4
						ECX = DSDWORD[EBX];
				}
				IF(ECX&true)
				{
						IF(xi>rw)rw=xi;
						___x = x+xi;
						IF(italic)___x+=math.ceil(ital);
						PixelRGB(___x,EDI);
						//_TMP_WEIGHT = 2;
						//WHILE(_TMP_WEIGHT)
						//{
						//	_TMP_WEIGHT--;
							IF(weight) PixelRGB(___x+1,EDI);
						//}
				}
				iii++;
            }
			IF(italic) ital-=size.offset_i;
			$inc yi
        }
        return rw;
}
:byte FONT::load(dword path)
{
	lib_init_fs();
	buffer_size = 0;
	use_smooth = true;
	IF(data)free(data);
	IF(!fs.read(path)) { debug("Error while loading font: "); debugln(path); return false; }
	begin = data = EAX;
	EBX = begin + ECX;
	$dec ebx
	height = DSBYTE[EBX];
	$dec ebx
	width = DSBYTE[EBX];
	block = math.ceil(height*width/32);
	return true;
}

:void FONT::prepare_buf(int x,y,w,h; dword text1)
{
	dword c, new_buffer_size;
	c = color;
	//left = x;
	IF(!text1)return;
	IF(size.text)IF(!changeSIZE())return;
	AX = c; r = AL; g = AH; c>>=16; AX = c; b = AL;
	getsize(text1);
	y -= size.offset.y;
	//top = y;
	size.width = w;
	size.height = h;

	new_buffer_size = w*h*3;
	IF(buffer_size!=new_buffer_size)
	{
		buffer_size = new_buffer_size; 
		free(buffer);
		buffer = malloc(buffer_size);
		EBX = bg_color;
		EDI = buffer;
		EAX = buffer_size+buffer;
		WHILE (EDI<EAX)
		{
			ESDWORD[EDI] = EBX;
			$add edi,3
		}
	}
	WHILE(DSBYTE[text1])
	{
		x+=symbol(x,y,DSBYTE[text1]);
		IF(weight)x+=math.ceil(size.text/17);
		text1++;
	}
	return;
}

#endif