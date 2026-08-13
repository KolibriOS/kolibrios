program Draw_Scaled_PNG_Image;
 
uses
  KolibriOS, LibImg;
 
var
  WndLeft, WndTop, WndWidth, WndHeight: Integer;
  
  ImageToDraw: PImage; // изображение, которое будем рисовать

// пробуем загрузить файл изображения, декодировать и масштабировать
function OpenImageFile(FilePath: PAnsiChar): PImage;
var
  FileAttributes: TFileAttributes;
  BytesRead: Integer;
  imgFile: Pointer;
  imgFileSize: Integer;
  imgData, imgScaled: PImage;
begin
  Result := nil;
  // если файл доступен
  if GetFileAttributes(FilePath, FileAttributes) = 0 then
  begin  
    // узнаём размер файла в байтах
    imgFileSize := FileAttributes.SizeLo;    
    // выделяем память для чтения туда этого файла
    GetMem(imgFile, imgFileSize);    
    // читаем файл
    ReadFile(FilePath, PAnsiChar(imgFile)^, imgFileSize, 0, 0, BytesRead);    
    // пробуем декодировать файл изображения
    imgData := img_decode(imgFile, imgFileSize, nil);    
    // освобождаем выделенную под файл память
    FreeMem(imgFile);    
    // если удалось декодировать файл изображения
    if imgData <> nil then
    begin      
      // если неподдерживаемая библиотекой глубина цвета      
      if not (imgData^.ImageType in [IMAGE_BPP24, IMAGE_BPP32, IMAGE_BPP8G]) then
      begin      
        // конвертируем в изображение с поддерживаемой глубиной
        imgScaled := img_convert(imgData, nil, IMAGE_BPP24, 0, 0);        
        // уничтожаем исходное изображение
        img_destroy(imgData);        
        imgData := imgScaled;
      end;      
      // пробуем масштабировать в новый размер 200x150
      imgScaled := img_scale(imgData, 0, 0, imgData^.Width, imgData^.Height, 
        nil, LIBIMG_SCALE_STRETCH, LIBIMG_INTER_DEFAULT, 200, 150);      
      Result := imgScaled;      
    end;
  end;  
end;

begin
  //SetAppDirAsCurrent;
  
  LibImg_initialization;
  
  with GetScreenSize do
  begin
    WndWidth := Width div 4;
    WndHeight := Height div 4;
    WndLeft := (Width - WndWidth) div 2;
    WndTop := (Height - WndHeight) div 2;
  end;
 
  ImageToDraw := OpenImageFile('./Background.png');
 
  while True do
    case WaitEvent of
      REDRAW_EVENT:
        begin
          BeginDraw;          
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'Draw Scaled PNG Image', $00FFFFFF,
            WS_SKINNED_FIXED + WS_CLIENT_COORDS + WS_CAPTION, CAPTION_MOVABLE);                              
          // если получилось масштабировать, то рисуем его в точке (10, 5)
          if ImageToDraw <> nil then
            img_draw(ImageToDraw, 10, 5, $ffffffff, $ffffffff, 0, 0);                    
          EndDraw;
        end;
      KEY_EVENT:
        GetKey;
      BUTTON_EVENT:
        if GetButton.ID = 1 then
          Exit;
    end;
end.
