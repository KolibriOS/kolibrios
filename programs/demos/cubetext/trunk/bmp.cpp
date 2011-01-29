//
//	bmp.cpp - source file / freeware
//
//	David Henry - tfc_duke@hotmail.com
//


#include	"bmp.h"
#include    <stdio.h>

#include    <libc/stubs.h>

extern "C"{
long filelength(int fhandle);
}

// --------------------------------------------------
// LoadFileBMP() - load a Windows/OS2 BITMAP image
//				   [.bmp].
//
// parameters :
//    - filename [in]  : image source file
//    - pixels	 [out] : 32 bits rgb image data
//    - width    [out] : image width in pixels
//    - height   [out] : image height in pixels
//    - flipvert [in]  : flip vertically
//
// return value :
//    - -1 : no image data
//    -  0 : failure
//    -  1 : success
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// accepted image formats :
//     # RGB 1-4-8-24-32 bits	WINDOWS - OS/2
//     # RLE 4-8 bits			WINDOWS
// --------------------------------------------------

int LoadFileBMP( const char *filename, unsigned char **pixels, int *width, int *height, bool flipvert )
{
	FILE		*file;			// file stream
	BITMAPFILEHEADER	*bmfh;			// bitmap file header
	BITMAPINFOHEADER	*bmih;			// bitmap info header (windows)
	BITMAPCOREHEADER	*bmch;			// bitmap core header (os/2)
	RGBTRIPLE			*os2_palette;	// pointer to the color palette os/2
	RGBQUAD				*win_palette;	// pointer to the color palette windows
	char				*buffer;		// buffer storing the entire file
	unsigned char		*ptr;			// pointer to pixels data
	int					bitCount;		// number of bits per pixel
	int					compression;	// compression type (rgb/rle)
	int					row, col, i;	// temporary variables
	int					w, h;			// width, height



	/////////////////////////////////////////////////////
	// read the entire file in the buffer

	file = fopen(filename,"rb");

	if( !file)
		return 0;

	long flen = filelength(fileno(file));

	buffer = new char[ flen + 1 ];
	int rd = fread(buffer, flen, 1, file);
	char *pBuff = buffer;

	fclose(file);

	/////////////////////////////////////////////////////
	

	// read the header
	bmfh = (BITMAPFILEHEADER *)pBuff;
	pBuff += sizeof( BITMAPFILEHEADER );

	// verify that it's a BITMAP file
	if( bmfh->bfType != BITMAP_ID )
	{
		delete [] buffer;
		return 0;
	}


	bmch = (BITMAPCOREHEADER *)pBuff;
	bmih = (BITMAPINFOHEADER *)pBuff;


	if( (bmih->biCompression < 0) || (bmih->biCompression > 3) )
	{
		// OS/2 style
		pBuff += sizeof( BITMAPCOREHEADER );

		bitCount	= bmch->bcBitCount;
		compression	= BI_OS2;

		w = bmch->bcWidth;
		h = bmch->bcHeight;
	}
	else
	{
		// WINDOWS style
		pBuff += sizeof( BITMAPINFOHEADER );

		bitCount	= bmih->biBitCount;
		compression	= bmih->biCompression;

		w = bmih->biWidth;
		h = bmih->biHeight;
	}


	if( width )
		*width	= w;

	if( height )
		*height	= h;


	if( !pixels )
	{
		delete [] buffer;
		return (-1);
	}


	/////////////////////////////////////////////////////
	// read the palette

	if( bitCount <= 8 )
	{
		// 24 and 32 bits images are not paletted

		// ajust the palette pointer to the memory in the buffer
		os2_palette = (RGBTRIPLE *)pBuff;
		win_palette = (RGBQUAD *)pBuff;

		//	[number of colors in the palette] * [size of one pixel]
		pBuff += (1 << bitCount) * (bitCount >> 3) * sizeof( unsigned char );
	}

	/////////////////////////////////////////////////////


	// allocate memory to store pixel data
	*pixels = new unsigned char[ w * h * 3 ];
	ptr		= &(*pixels)[0];


	// move the pixel data pointer to the begening of bitmap data
	pBuff = buffer + (bmfh->bfOffBits * sizeof( char ));


	/////////////////////////////////////////////////////
	// read pixel data following the image compression
	// type and the number of bits per pixels
	/////////////////////////////////////////////////////

	switch( compression )
	{
		case BI_OS2:
		case BI_RGB:
		{
			for( row = h - 1; row >= 0; row-- )
			{
				if( flipvert )
					ptr = &(*pixels)[ row * w * 3 ];

				switch( bitCount )
				{
					case 1:
					{
// RGB 1 BITS
						for( col = 0; col < (int)(w / 8); col++ )
						{
							// read the current pixel
							unsigned char color = *((unsigned char *)(pBuff++));

							for( i = 7; i >= 0; i--, ptr += 3 )
							{
								// convert indexed pixel (1 bit) into rgb (32 bits) pixel
								int clrIdx = ((color & (1<<i)) > 0);

								if( compression == BI_OS2 )
								{
									ptr[2] = os2_palette[ clrIdx ].rgbtRed;
									ptr[1] = os2_palette[ clrIdx ].rgbtGreen;
									ptr[0] = os2_palette[ clrIdx ].rgbtBlue;									
								}
								else
								{
									ptr[2] = win_palette[ clrIdx ].rgbRed;
									ptr[1] = win_palette[ clrIdx ].rgbGreen;
									ptr[0] = win_palette[ clrIdx ].rgbBlue;
								}
							}
						}

						break;
					}

					case 4:
					{
// RGB 4 BITS
						for( col = 0; col < (int)(w / 2); col++, ptr += 6 )
						{
							// read the current pixel
							unsigned char color = *((unsigned char *)(pBuff++));

							// convert indexed pixel (4 bits) into rgb (32 bits) pixel
							int clrIdx;

							if( compression == BI_OS2 )
							{
								clrIdx = (color >> 4);
								ptr[2] = os2_palette[ clrIdx ].rgbtRed;
								ptr[1] = os2_palette[ clrIdx ].rgbtGreen;
								ptr[0] = os2_palette[ clrIdx ].rgbtBlue;

								clrIdx = (color & 0x0F);
								ptr[6] = os2_palette[ clrIdx ].rgbtRed;
								ptr[5] = os2_palette[ clrIdx ].rgbtGreen;
								ptr[4] = os2_palette[ clrIdx ].rgbtBlue;

							}
							else
							{
								clrIdx = (color >> 4);
								ptr[2] = win_palette[ clrIdx ].rgbRed;
								ptr[1] = win_palette[ clrIdx ].rgbGreen;
								ptr[0] = win_palette[ clrIdx ].rgbBlue;

								clrIdx = (color & 0x0F);
								ptr[6] = win_palette[ clrIdx ].rgbRed;
								ptr[5] = win_palette[ clrIdx ].rgbGreen;
								ptr[4] = win_palette[ clrIdx ].rgbBlue;
							}
						}

						break;
					}

					case 8:
					{
// RGB 8 BITS
						for( col = 0; col < w; col++, ptr += 3 )
						{
							// read the current pixel
							unsigned char color = *((unsigned char *)(pBuff++));

							// convert indexed pixel (8 bits) into rgb (32 bits) pixel
							if( compression == BI_OS2 )
							{
								ptr[2] = os2_palette[ color ].rgbtRed;
								ptr[1] = os2_palette[ color ].rgbtGreen;
								ptr[0] = os2_palette[ color ].rgbtBlue;
							}
							else
							{
								ptr[2] = win_palette[ color ].rgbRed;
								ptr[1] = win_palette[ color ].rgbGreen;
								ptr[0] = win_palette[ color ].rgbBlue;
							}
						}

						break;
					}

					case 24:
					{
// RGB 24 BITS
						for( col = 0; col < w; col++, ptr += 3 )
						{
							// convert bgr pixel (24 bits) into rgb (32 bits) pixel
							RGBTRIPLE *pix = (RGBTRIPLE *)pBuff;
							pBuff += sizeof( RGBTRIPLE );

							ptr[2] = pix->rgbtRed;
							ptr[1] = pix->rgbtGreen;
							ptr[0] = pix->rgbtBlue;
						}

  						break;
					}

					case 32:
					{
// RGB 32 BITS
						for( col = 0; col < w; col++, ptr += 3 )
						{
							// // convert bgr pixel (32 bits) into rgb (32 bits) pixel
							RGBQUAD *pix = (RGBQUAD *)pBuff;
							pBuff += sizeof( RGBQUAD );

							ptr[2] = pix->rgbRed;
							ptr[1] = pix->rgbGreen;
							ptr[0] = pix->rgbBlue;

						}

						break;
					}
				}
			}

			break;
		}

		case BI_RLE8:
		{
// RLE 8 BITS
			for( row = h - 1; row >= 0; row-- )
			{
				if( flipvert )
					ptr = &(*pixels)[ row * w * 3 ];

				for( col = 0; col < w; /* nothing */ )
				{
					// get one packet (2 bytes)
					unsigned char byte1 = *((unsigned char *)(pBuff++));
					unsigned char byte2 = *((unsigned char *)(pBuff++));


					if( byte1 == RLE_COMMAND )
					{
						// absolute encoding
						for( i = 0; i < byte2; i++, ptr += 3, col++ )
						{
							// read the current pixel
							unsigned char color = *((unsigned char *)(pBuff++));

							// convert indexed pixel (8 bits) into rgb (32 bits) pixel
							ptr[2] = win_palette[ color ].rgbRed;
							ptr[1] = win_palette[ color ].rgbGreen;
							ptr[0] = win_palette[ color ].rgbBlue;
						}

						if( (byte2 % 2) == 1 )
							pBuff++;
					}
					else
					{
						// read next pixels
						for( i = 0; i < byte1; i++, ptr += 3, col++ )
						{
							// convert indexed pixel (8 bits) into rgb (32 bits) pixel
							ptr[2] = win_palette[ byte2 ].rgbRed;
							ptr[1] = win_palette[ byte2 ].rgbGreen;
							ptr[0] = win_palette[ byte2 ].rgbBlue;
						}        
					}
				}
			}

			break;
		}

		case BI_RLE4:
		{
// RLE 4 BITS
			unsigned char color;
			int	bytesRead = 0;		// number of bytes read

			for( row = h - 1; row >= 0; row-- )
			{
				if( flipvert )
					ptr = &(*pixels)[ row * w * 3 ];

				for( col = 0; col < w; /* nothing */ )
				{
					// get one packet (2 bytes)
					unsigned char byte1 = *((unsigned char *)(pBuff++));
					unsigned char byte2 = *((unsigned char *)(pBuff++));

					bytesRead += 2;


					if( byte1 == RLE_COMMAND )
					{
						// absolute encoding
						unsigned char databyte;

						for( i = 0; i < byte2; i++, ptr += 3, col++ )
						{
							if( (i % 2) == 0 )
							{
								// read the current pixel
								databyte = *((unsigned char *)(pBuff++));
								bytesRead++;

								color = (databyte >> 4);	// 4 first bits
							}
							else
							{
								color = (databyte & 0x0F);	// 4 last bits
							}

							// convert indexed pixel (4 bits) into rgb (32 bits) pixel
							ptr[2] = win_palette[ color ].rgbRed;
							ptr[1] = win_palette[ color ].rgbGreen;
							ptr[0] = win_palette[ color ].rgbBlue;
						}
          
						while( (bytesRead % 2) != 0 )
						{
							pBuff++;
							bytesRead++;
						}
					}
					else
					{
						// read next pixels
						for( i = 0; i < byte1; i++, ptr += 3, col++ )
						{
							if( (i % 2) == 0 )
								color = (byte2 >> 4);	// 4 first bits
							else
								color = (byte2 & 0x0F);	// 4 last bits

							// convert indexed pixel (4 bits) into rgb (32 bits) pixel
							ptr[2] = win_palette[ color ].rgbRed;
							ptr[1] = win_palette[ color ].rgbGreen;
							ptr[0] = win_palette[ color ].rgbBlue;
						}        
					}
				}
			}

			break;
		}
	}


	// free buffer memory
	delete [] buffer;


	// return success
	return 1;
}
