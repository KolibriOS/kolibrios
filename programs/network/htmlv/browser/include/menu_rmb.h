//Leency - 2012

#define ITEM_HEIGHT 18
#define ITEM_WIDTH  138

char *ITEMS_LIST[]={
"View html code    F3",
"--------------------", //сделать определение таких линий и рисовать их
"KOI-8         Ctrl+K",
"UTF           Ctrl+U",
0}; 

proc_info MenuForm;

	
void menu_rmb()
{

	mouse mm;
	int items_num;
	int id, letitclose=0;
	
	SetEventMask(100111b); 
	
	loop() switch(CheckEvent())
	{
		case evMouse:
				/*mm.get();
				//кульно
				if (mm.x>85) && (mm.x<155) && (mm.y>190) && (mm.y<190+22)
				if (mm.lkm) {DrawRegion_3D(86,191,68,20,0xC7C7C7,0xFFFFFF); letitclose=1;}
				ELSE {IF (letitclose) {DrawRegion_3D(86,191,68,20,0xFFFFFF,0xC7C7C7); Pause(7); ExitProcess();}}
				ELSE IF (letitclose) {letitclose=0; DrawRegion_3D(86,191,68,20,0xFFFFFF,0xC7C7C7);}*/ 
				break;
				
		case evButton: 
				id=GetButtonID();
				if (id==1) ExitProcess();
				if (id==10) WB1.Scan(52); //View html code
				if (id==11) break;
				if (id==12) WB1.Scan(11); //KOI
				if (id==13) WB1.Scan(21); //UTF
				
				ExitProcess();
				break;
				
		case evKey:
				if (GetKey()==27) ExitProcess();
				break;
				
		case evReDraw:
				for (items_num=0; ITEMS_LIST[items_num]<>0; items_num++;) {};
				DefineAndDrawWindow(Form.left+m.x,Form.top+m.y+GetSkinWidth()+3,ITEM_WIDTH,items_num*ITEM_HEIGHT,0x01,0x10FFFFFF,0,0,0);

				DrawRegion(0,0,ITEM_WIDTH,items_num*ITEM_HEIGHT,0x777777); //ободок
				DrawBar(1,1,ITEM_WIDTH-1,items_num*ITEM_HEIGHT-1,0xFFFFFF); //фон
				for (i=0; i<items_num; i++;)
				{
					DefineButton(0, i*ITEM_HEIGHT, ITEM_WIDTH, ITEM_HEIGHT, i+10+BT_HIDE, 0xFFFFFF);
					WriteText(8,i*ITEM_HEIGHT+6,0x80,0x000000,ITEMS_LIST[i],0);
				}
		default:
				GetProcessInfo(#MenuForm, SelfInfo);
				id=GetSlot(MenuForm.ID);
				if (id<>ActiveProcess()) ExitProcess();			
	}
}