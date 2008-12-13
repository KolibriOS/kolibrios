#pragma pack(push, 1)
typedef struct sTGAHeader
{
  Byte  BytesInIdentField;
  Byte  ColorMapType;       // Color map type - 0 [no map] 1 [256 entry]
  Byte  ImageTypeCode;      /* Image type
                               [0] No image data included
                               [1] Uncompressed color map image [4,6]
                               [2] Uncompressed RGB Image [16,24]
                               [3] Uncompressed Black & White
                               [9] RLE Color map image
                               [10] RLE RGB Image
                               [11] RLE Black & White
                               [32 | 33] compressed color map by Huffman, Delta, RLE
                            */
  Word  ColorMapOrigin;     // Offset of first color map entry
  Word  ColorMapLength;     // Number of color map entries
  Byte  ColorMapEntrySize;  // Number of bits per color map entries
  Word  XOrigin;
  Word  YOrigin;
  Word  Width;
  Word  Height;
  Byte  ImagePixelSize;     // BPP
  Byte  ImageDescByte;      // Flags
} sTGAHeader;
#pragma pack(pop)

class TGAFile
{
protected:
  sTGAHeader Tga_head;
public:
  Word  width;
  Word  height;
  Byte* buffer;
  int   LoadTGAFile(Byte* filebuff, Dword filesize);
};