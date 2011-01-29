//
//	bmp.h - header file / freeware
//
//	David Henry - tfc_duke@hotmail.com
//

#ifndef		__BITMAP_H_
#define		__BITMAP_H_



// magic number "BM"
#define			BITMAP_ID			('B' + ('M'<<8))



// header byte type for RLE
#define			RLE_COMMAND			0
#define			RLE_ENDOFLINE		0
#define			RLE_ENDOFBITMAP		1
#define			RLE_DELTA			2

#define			BI_OS2				-1


// compression type
#define			BI_RGB				0
#define			BI_RLE8				1
#define			BI_RLE4				2
#define			BI_BITFIELDS		3



#pragma warning( disable : 4103 )



// --------------------------------------------
// tagBITMAPFILEHEADER - bitmap file header.
// --------------------------------------------

#pragma pack(2)

typedef struct tagBITMAPFILEHEADER		// bmfh
{
	unsigned short	bfType;				// magic number "BM"
	unsigned int	bfSize;				// file size
	unsigned short	bfReserved1;		// reserved
	unsigned short	bfReserved2;		// reserved
	unsigned int	bfOffBits;			// offset to bitmap data

} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

#pragma pack(4)



// --------------------------------------------
// tagBITMAPCOREHEADER - bitmap core header.
// --------------------------------------------

typedef struct tagBITMAPCOREHEADER		// bmch
{
	unsigned int	bcSize;				// size of the structure
	unsigned short	bcWidth;			// image width
	unsigned short	bcHeight;			// image height
	unsigned short	bcPlanes;			// must be equal to 1
	unsigned short	bcBitCount;			// number of bits per pixel

} BITMAPCOREHEADER, *PBITMAPCOREHEADER;



// --------------------------------------------
// tagRGBTRIPLE - 24 bits pixel
// --------------------------------------------

typedef struct tagRGBTRIPLE			// rgbt
{
	unsigned char	rgbtBlue;		// blue
	unsigned char	rgbtGreen;		// green
	unsigned char	rgbtRed;		// red

} RGBTRIPLE, *PRGBTRIPLE;



// --------------------------------------------
// tagRGBQUAD - 32 bits pixel
// --------------------------------------------

typedef struct tagRGBQUAD			// rgbt
{
	unsigned char	rgbBlue;		// blue
	unsigned char	rgbGreen;		// green
	unsigned char	rgbRed;			// red
	unsigned char	rgbReserved;	// reserved

} RGBQUAD, *PRGBQUAD;



// --------------------------------------------
// tagBITMAPCOREINFO - bitmap core info.
// --------------------------------------------

typedef struct tagBITMAPCOREINFO		// bmci
{
	BITMAPCOREHEADER	bmciHeader;		// size of the structure
	RGBTRIPLE			bcmiColors[1];	// color palette

} BITMAPCOREINFO, *PBITMAPCOREINFO;


// --------------------------------------------
// BITMAPFILEHEADER - bitmap info header.
// --------------------------------------------

typedef struct tagBITMAPINFOHEADER
{
	unsigned int	biSize;				// size of the structure
	int				biWidth;			// image width
	int				biHeight;			// image height
	unsigned short	biPlanes;			// must be equal to 1
	unsigned short	biBitCount;			// number of bits per pixel
	unsigned int	biCompression;		// compression type
	unsigned int	biSizeImage;		// size of data bitmap
	int				biXPelsPerMeter;	// number of pixels per meter on the X axis
	int				biYPelsPerMeter;	// number of pixels per meter on the Y axis
	unsigned int	biClrUsed;			// number of colors used
	unsigned int	biClrImportant;		// number of important colors

} BITMAPINFOHEADER, *PBITMAPINFOHEADER;


     
// prototype
int LoadFileBMP( const char *filename, unsigned char **pixels, int *width, int *height, bool flipvert );
 


#endif // __BITMAP_H_
