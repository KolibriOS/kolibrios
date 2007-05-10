; Interface

macro Window xStart,yStart,xSize,ySize,bColor,gColor,fColor
{
 mov ebx,xStart
 shl ebx,16
 add ebx,xSize
 mov ecx,yStart
 shl ecx,16
 add ecx,ySize
 mov edx,bColor
 mov esi,gColor
 mov edi,fColor
 xor eax,eax
 mcall
}


;WriteTextToWindow
macro Text xStart,yStart,rgbColor,pText,nTextLen
{
 mov ebx,xStart
 shl ebx,16
 add ebx,yStart
 mov ecx,rgbColor
 mov edx,pText
 mov esi,nTextLen
 mov eax,4
 mcall
}

;DisplayNumber
macro Number xStart,yStart,nPrintType,noOfDigits,Data,rgbColor
{

 mov edx,xStart
 shl edx,16
 add edx,yStart
 mov ebx,noOfDigits
 shl ebx,16
 or ebx,nPrintType
 mov ecx,Data
 mov esi,rgbColor
 mov eax,47
 mcall
}

macro DrawLine xStart,xEnd,yStart,yEnd,rgbColor
{
 mov ebx,xStart
 shl ebx,16
 add ebx,xEnd
 mov ecx,yStart
 shl ecx,16
 add ecx, yEnd
 mov edx,rgbColor
 mov eax,38
 mcall
}

macro PutImage xPos,yPos,xImage,yImage,pImage
{
 mov ecx,xImage
 shl ecx,16
 add ecx, yImage
 mov edx,xPos
 shl edx,16
 add edx,yPos
 mov ebx,pImage
 mov eax,7
 mcall
}

macro Button xStart,yStart,xSize,ySize,nID,rgbColor
{
 mov ebx,xStart
 shl ebx,16
 add ebx,xSize
 mov ecx,yStart
 shl ecx,16
 add ecx,ySize
 mov edx,nID
 mov esi,rgbColor
 mov eax,8
 mcall
}

macro CreateTread EntryPoint,StackPos
{
 xor ebx,ebx
 inc ebx
 mov ecx,EntryPoint
 mov edx,StackPos
 mov eax,51
 mcall
}
