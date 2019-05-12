#pragma pack(push, 1)
typedef struct PCXHeader
{
  Byte   bManufacturer;
  Byte   bVersion;
  Byte   bEncoding;
  Byte   bBPP;
  Word   dwX;
  Word   dwY;
  Word   dwWidth;
  Word   dwHeight;
  Word   dwHRes;
  Word   dwVRes;
  Byte   colormap[48];
  Byte   bReserved;
  Byte   bNPlanes;
  Word   iBPL;
  Word   iPalInfo;  
} PCXHeader;
#pragma pack(pop)

class PCXFile
{
protected:
  PCXHeader Pcx_head;
public:
  Word  width;
  Word  height;
  Byte* buffer;
  int   LoadPCXFile(Byte* filebuff, Dword filesize);
};