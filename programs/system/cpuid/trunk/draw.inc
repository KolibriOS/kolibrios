; Interface

macro Window xStart,yStart,xSize,ySize,bColor,gColor,fColor
{
 __mov ebx,xStart,xSize
 __mov ecx,yStart,ySize
 __mov edx,bColor
 __mov esi,gColor
 __mov edi,fColor
 xor eax,eax
 mcall
}


;WriteTextToWindow
macro Text xStart,yStart,rgbColor,pText,nTextLen
{
 __mov ebx,xStart,yStart
 __mov ecx,rgbColor
 __mov edx,pText
 __mov esi,nTextLen
 mov eax,4
 mcall
}

;DisplayNumber
macro Number xStart,yStart,nPrintType,noOfDigits,Data,rgbColor
{
 __mov edx,xStart,yStart
 __mov ebx,noOfDigits,nPrintType
 __mov ecx,Data
 __mov esi,rgbColor
 mov eax,47
 mcall
}

macro DrawLine xStart,xEnd,yStart,yEnd,rgbColor
{
 __mov ebx,xStart,xEnd
 __mov ecx,yStart,yEnd
 __mov edx,rgbColor
 mov eax,38
 mcall
}

macro PutImage xPos,yPos,xImage,yImage,pImage
{
 __mov ecx,xImage,yImage
 __mov edx,xPos,yPos
 __mov ebx,pImage
 mov eax,7
 mcall
}

macro Button xStart,yStart,xSize,ySize,nID,rgbColor
{
 __mov ebx,xStart,xSize
 __mov ecx,yStart,ySize
 __mov edx,nID
 __mov esi,rgbColor
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
