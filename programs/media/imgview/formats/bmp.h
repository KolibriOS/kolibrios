#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER 
{
  Word   bfType;       //тип файла (для битового образа - BM)
  Dword  bfSize;       //размер файла в dword
  Word   bfReserved1;  //не используется
  Word   bfReserved2;  //не используется
  Dword  bfOffbits;    //смещение данных битового образа от заголовка в байтах
} tagBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
  Dword  biSize;          //число байт, занимаемых структурой BITMAPINFOHEADER
  Dword  biWidth;         //ширина битового образа в пикселах
  Dword  biHeight;        //высота битового образа в пикселах 
  Word   biPlanes;	  //число битовых плоскостей устройства
  Word   biBitCount;	  //число битов на пиксель
  Dword  biCompression;	  //тип сжатия
  Dword  biSizeImage;	  //размер картинки в байтах
  Dword  biXPelsPerMeter; //горизонтальное разрешение устройства, пиксел/м
  Dword  biYPelPerMeter;  //вертикальное разрешение устройства, пиксел/м
  Dword  biClrUsed;       //число используемых цветов
  Dword  biClrImportant;  //число "важных" цветов
} tagBITMAPINFOHEADER;

typedef struct tagRGBQUAD
{
  Byte   rgbBlue;
  Byte   rgbGreen;
  Byte   rgbRed;
  Byte   rgbReserved;  
} tagRGBQUAD;
#pragma pack(pop)

class BMPFile
{
protected:
  tagBITMAPFILEHEADER Bmp_head;
  tagBITMAPINFOHEADER Info_head;
public:
  Dword width;
  Dword height;
  Byte* buffer;
  int  BMPFile::LoadBMPFile(Byte* filebuff, Dword filesize);
};