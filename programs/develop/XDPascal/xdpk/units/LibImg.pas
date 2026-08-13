unit LibImg;

interface

uses
  KolibriOS, Libs;

// Список идентификаторов форматов
const
  LIBIMG_FORMAT_BMP  = 1;
  LIBIMG_FORMAT_ICO  = 2;
  LIBIMG_FORMAT_CUR  = 3;
  LIBIMG_FORMAT_GIF  = 4;
  LIBIMG_FORMAT_PNG  = 5;
  LIBIMG_FORMAT_JPEG = 6;
  LIBIMG_FORMAT_TGA  = 7;
  LIBIMG_FORMAT_PCX  = 8;
  LIBIMG_FORMAT_XCF  = 9;
  LIBIMG_FORMAT_TIFF = 10;
  LIBIMG_FORMAT_PNM  = 11;
  LIBIMG_FORMAT_WBMP = 12;
  LIBIMG_FORMAT_XBM  = 13;
  LIBIMG_FORMAT_Z80  = 14;

// Типы масштабирования         // соответствующие параметры для img_scale:
  LIBIMG_SCALE_NONE       = 0;  // не масштабировать
  LIBIMG_SCALE_INTEGER    = 1;  // коэффициент масштабирования; зарезервировано 0
  LIBIMG_SCALE_TILE       = 2;  // новая ширина; новая высота
  LIBIMG_SCALE_STRETCH    = 3;  // новая ширина; новая высота
  LIBIMG_SCALE_FIT_BOTH   = LIBIMG_SCALE_STRETCH;
  LIBIMG_SCALE_FIT_MIN    = 4;  // новая ширина; новая высота
  LIBIMG_SCALE_FIT_RECT   = LIBIMG_SCALE_FIT_MIN;
  LIBIMG_SCALE_FIT_WIDTH  = 5;  // новая ширина; новая высота
  LIBIMG_SCALE_FIT_HEIGHT = 6;  // новая ширина; новая высота
  LIBIMG_SCALE_FIT_MAX    = 7;  // новая ширина; новая высота

// Алгоритм интерполяции
  LIBIMG_INTER_NONE     = 0;  // использовать с LIBIMG_SCALE_INTEGER, LIBIMG_SCALE_TILE и т.д.
  LIBIMG_INTER_BILINEAR = 1;
  // LIBIMG_INTER_BICUBIC   = 2;
  // LIBIMG_INTER_LANCZOS   = 3;
  LIBIMG_INTER_DEFAULT  = LIBIMG_INTER_BILINEAR;

// Коды ошибок
  LIBIMG_ERROR_OUT_OF_MEMORY     = 1;
  LIBIMG_ERROR_FORMAT            = 2;
  LIBIMG_ERROR_CONDITIONS        = 3;
  LIBIMG_ERROR_BIT_DEPTH         = 4;
  LIBIMG_ERROR_ENCODER           = 5;
  LIBIMG_ERROR_SRC_TYPE          = 6;
  LIBIMG_ERROR_SCALE             = 7;
  LIBIMG_ERROR_INTER             = 8;
  LIBIMG_ERROR_NOT_INPLEMENTED   = 9;
  LIBIMG_ERROR_INVALID_INPUT     = 10;

// Флаги кодирования
  // LIBIMG_ENCODE_STRICT_SPECIFIC = $01;
  LIBIMG_ENCODE_STRICT_BIT_DEPTH = $02;
  // LIBIMG_ENCODE_DELETE_ALPHA    = $08;
  // LIBIMG_ENCODE_FLUSH_ALPHA     = $10;

// Значения для Image.ImageType
// должны быть последовательными для быстрого переключения в функциях поддержки
  IMAGE_BPP8I = 1;   // индексированный
  IMAGE_BPP24 = 2;
  IMAGE_BPP32 = 3;
  IMAGE_BPP15 = 4;
  IMAGE_BPP16 = 5;
  IMAGE_BPP1  = 6;
  IMAGE_BPP8G = 7;   // оттенки серого
  IMAGE_BPP2I = 8;
  IMAGE_BPP4I = 9;
  IMAGE_BPP8A = 10;  // оттенки серого с альфа-каналом; только уровень приложения!

// Биты в Image.Flags
  IMAGE_IS_ANIMATED = 1;

// Флаги отражения
  FLIP_VERTICAL   = $01;
  FLIP_HORIZONTAL = $02;
  FLIP_BOTH       = FLIP_VERTICAL or FLIP_HORIZONTAL;

// Флаги поворота
  ROTATE_90_CW   = $01;
  ROTATE_180     = $02;
  ROTATE_270_CW  = $03;
  ROTATE_90_CCW  = ROTATE_270_CW;
  ROTATE_270_CCW = ROTATE_90_CW;

type
  // Указатели
  PFormatsTableEntry = ^TFormatsTableEntry;
  PImage = ^TImage;
  PImageDecodeOptions = ^TImageDecodeOptions;

  // Типы функций для работы с форматами
  TFormatIsFunction = function(Data: Pointer; Length: Integer): Integer stdcall;
  TFormatDecodeFunction = function(Data: Pointer; Length: Integer; Options: PImageDecodeOptions): PImage stdcall;
  TFormatEncodeFunction = function(Img: PImage; Common: Integer; Specific: Pointer): Pointer stdcall;

  TFormatsTableEntry = packed record
    FormatId: Integer;
    IsFormat: TFormatIsFunction;      // функция проверки формата
    Decode: TFormatDecodeFunction;    // функция декодирования
    Encode: TFormatEncodeFunction;    // функция кодирования
    Capabilities: Integer;           // возможности формата (битовая маска)
  end;

  TImage = packed record
    Checksum: Integer;   // ((Width ROL 16) OR Height) XOR Data[0]; пока игнорируется
    Width: Integer;
    Height: Integer;
    Next: PImage;         // указатель на следующее изображение
    Previous: PImage;     // указатель на предыдущее изображение
    ImageType: Integer;  // один из IMAGE_BPPN
    Data: Pointer;        // указатель на данные изображения
    Palette: Pointer;     // используется если Type = IMAGE_BPP1, IMAGE_BPP2, IMAGE_BPP4 или IMAGE_BPP8I
    Extended: Pointer;    // расширенные данные
    Flags: Integer;      // битовое поле
    Delay: Integer;      // используется если IMAGE_IS_ANIMATED установлен в Flags
  end;

  TImageDecodeOptions = packed record
    UsedSize: Integer;        // если >=8, поле BackgroundColor действительно, и так далее
    BackgroundColor: Integer; // используется для прозрачных изображений как фон
  end;

var
  hLibImg: Pointer;

  // Основные функции библиотеки
  img_is_img: function(Data: Pointer; Length: Integer): Integer stdcall;
  img_count: function(Img: PImage): Integer stdcall;
  img_decode: function(Data: Pointer; Length: Integer; Options: PImageDecodeOptions): PImage stdcall;
  img_encode: function(Img: PImage; Common: Integer; Specific: Pointer): Pointer stdcall;
  img_draw: procedure(Img: PImage; X, Y, Width, Height, XPos, YPos: Integer) stdcall;

  // Функции создания и удаления изображений
  img_create: function(Width, Height, ImgType: Integer): PImage stdcall;
  img_destroy: function(Img: PImage): Boolean stdcall;

  // Функции преобразования и обработки изображений
  img_to_rgb2: procedure(Img: PImage; Output: Pointer) stdcall;
  
  {* в данный момент функции не сохраняют EBX — это ошибка библиотеки! *}
  //img_flip: function(Img: PImage; FlipKind: Integer): Boolean stdcall;
  //img_rotate: function(Img: PImage; RotateKind: Integer): Boolean stdcall;
  //*********************************************************************
  
  img_convert: function(Src: PImage; Dst: PImage; DstType: Integer; Flags: Integer; Param: Integer): PImage stdcall;
  img_scale: function(Src: PImage; CropX, CropY, CropWidth, CropHeight: Integer; Dst: PImage; Scale, Inter, Param1, Param2: Integer): PImage stdcall;

  procedure LibImg_initialization;
  
implementation

procedure LibImg_initialization;
begin
  hLibImg := LoadLibrary('/sys/lib/libimg.obj');
  img_is_img := GetProcAddress(hLibImg, 'img_is_img');
  img_count := GetProcAddress(hLibImg, 'img_count');
  img_decode := GetProcAddress(hLibImg, 'img_decode');
  img_encode := GetProcAddress(hLibImg, 'img_encode');
  img_create := GetProcAddress(hLibImg, 'img_create');
  img_destroy := GetProcAddress(hLibImg, 'img_destroy');
  img_to_rgb2 := GetProcAddress(hLibImg, 'img_to_rgb2');
  
  {* в данный момент функции не сохраняют EBX — это ошибка библиотеки! *}
  //img_flip := GetProcAddress(hLibImg, 'img_flip');
  //img_rotate := GetProcAddress(hLibImg, 'img_rotate');
  //*********************************************************************
  
  img_convert := GetProcAddress(hLibImg, 'img_convert');
  img_draw := GetProcAddress(hLibImg, 'img_draw');
  img_scale := GetProcAddress(hLibImg, 'img_scale');
  InitLibrary(GetProcAddress(hLibImg, 'lib_init'));
end;

end.