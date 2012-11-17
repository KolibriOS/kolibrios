//Leency - 2012

dword onLeft(dword right,left) {EAX=Form.width-right-left;}
dword onTop(dword down,up) {EAX=Form.height-GetSkinHeight()-down-up;}


void ShowMessage(dword message)
{
	DrawFlatButton(Form.width/2-13,160,200,80,0,0xFFB6B5, message);
	Pause(150);
	List_ReDraw();
}


dword ConvertSize(dword bytes)
{
	unsigned char size_prefix[8], size_nm[4];
	if (bytes>=1073741824) copystr(" Gb",#size_nm);
	else if (bytes>=1048576) copystr(" Mb",#size_nm);
	else if (bytes>=1024) copystr(" Kb",#size_nm);
	else copystr(" b ",#size_nm);
	while (bytes>1023) bytes/=1024;
	copystr(IntToStr(bytes),#size_prefix);
	copystr(#size_nm,#size_prefix+strlen(#size_prefix));
	return #size_prefix;
}

inline fastcall void TVScroll() { //Прокрутка
	dword on_y;
	if (count<=0) {DrawFlatButton(onLeft(27,0),57,16,onTop(22,58),0,0xE4DFE1,""); return;}
	on_y = za_kadrom * onTop(22,57) / count +57;
	scroll_size=onTop(22,57) * f_visible - f_visible / count;
	if (scroll_size<20) scroll_size = 20; //устанавливаем минимальный размер скролла
	if (scroll_size>onTop(22,57)-on_y+56) || (za_kadrom+f_visible>=count) on_y=onTop(23+scroll_size,0); //для большого списка 
	DrawFlatButton(onLeft(27,0),on_y,16,scroll_size,0,0xE4DFE1,"");//ползунок
	DrawBar(onLeft(26,0),57,15,on_y-57,0xCED0D0);//поле до ползунка
	DrawBar(onLeft(26,0),on_y+scroll_size+1,15,onTop(22,57)-scroll_size-on_y+56,0xCED0D0); //поле после ползунка
}