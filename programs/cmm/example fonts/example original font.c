#define MEMSIZE 0x7E80

#include "../lib/font.h"

byte id,key;

FONT TimeNewRoman = 0;
FONT Verdana = 0;
void main()
{   

	SetEventMask(1100111b);
	
	Verdana.load("font/Verdana 15px original");
	font.load("font/Georgia 45px original");
	TimeNewRoman.load("font/Times New Roman 30px original");
	loop()
   {
      switch(WaitEvent())
      {
		case evMouse:
			mouse.get();
		
			
		break;
         case evButton:
            id=GetButtonID();               
            if (id==1) ExitProcess();
			break;
      
        case evKey:
			key = GetKey();
			if (key==013){ //Enter
				draw_window();
				
			}
			break;
         
         case evReDraw:
			draw_window();

			break;
      }
   }
}
void draw_window()
{
	proc_info Form;
	
	int i =0;
	DefineAndDrawWindow(215,100,450,500,0x33,0xFFFFFF,"Window header");
	GetProcessInfo(#Form, SelfInfo);
	
	font.text(0,0,"Georgia 45px original",0x5522DD);
	font.text(3,43,"Съешь еще этих мягких французких булок, да выпей чаю. 1234567890",0xDEDEDE);
	font.text(0,40,"Съешь еще этих мягких французких булок, да выпей чаю. 1234567890",0x0);
	
	
	font.text(0,80,"Times New Roman 30px original",0x993366);
	TimeNewRoman.text(3,113,"Съешь еще этих мягких французких булок, да выпей чаю. 1234567890",0xDEDEDE);
	TimeNewRoman.text(0,110,"Съешь еще этих мягких французких булок, да выпей чаю. 1234567890",0x0);
	
	Verdana.textarea(0,150,"Этот пример демонстрирует возможности шрифта, \rкоторый создан специально для колибриОС.\rВ ближайшее время активно добавляются шрифты.\rРедактировать шрифты можно на сайте: http://font.ga.\rДля редактирования шрифта, шрифт должен быть в несжатом виде,\rтакже соответствовать стандарту!\rВ планах сделать компактный файл,\rкоторый будет содержать разные размеры одного шрифта.\rОригинальный файл (т.е.) не сжатый файл много занимает места,\rпоэтому рекомендую сжимать kpack. Шрифты на заказ https://vk.com/pavelyakov39\rПрорисовка пока что медленная - временно.Кодировка шрифта ANSII.",0x5522DD);
	Verdana.text(30,350,"А теперь...",0xAA4444);
	font.text(60,370,"До встречи!!!",0xDD4444);
}