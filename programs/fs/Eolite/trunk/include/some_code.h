//Leency - 2012

dword onLeft(dword right,left) {EAX=Form.width-right-left;}
dword onTop(dword down,up) {EAX=Form.height-GetSkinWidth()-down-up;}


void ShowMessage(dword message)
{
	DrawFlatButton(Form.width/2-13,160,200,80,0,0xFFB6B5, message);
	Pause(150);
	List_ReDraw();
}


dword ConvertSize(dword bytes)
{
	byte size_prefix[8], temp[3];
	IF (bytes>=1073741824) copystr(" Gb",#temp);
	ELSE IF (bytes>=1048576) copystr(" Mb",#temp);
	ELSE IF (bytes>=1024) copystr(" Kb",#temp);
	ELSE copystr(" b ",#temp);
	WHILE (bytes>1023) bytes/=1024;
	copystr(IntToStr(bytes),#size_prefix);
	copystr(#temp,#size_prefix+strlen(#size_prefix));
	EAX=#size_prefix;
}
