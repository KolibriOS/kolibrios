#include <kolibri.h>
#include <kos_heap.h>
#include <kos_file.h>
#include <kos_unpack.h>
#include <load_lib.h>
#include <l_proc_lib.h>
#include <l_tinygl.h>
#include <l_box_lib.h>

using namespace Kolibri;

const char header[] = "Blocks";
char library_path[2048];

OpenDialog_data ofd;
unsigned char procinfo[1024];
char plugin_path[4096], openfile_path[4096], filename_area[256];
od_filter filter1 = { 7, "BJS\0\0" };

namespace Kolibri{
	char CurrentDirectoryPath[2048];
}

struct BlockList{
	unsigned char* name;
	long int id_l, p_cou;
	float* vert_d;
	float* norm_d;
};

BlockList* b_list = 0;
long int b_count;

unsigned char* b_data = 0;
unsigned char* f_data = 0;

struct ColorList{
	unsigned char* name;
	long int color;
};

const long C_COUNT_MAX = 32;
const long TOOLBAR_H = 29;
ColorList c_list[C_COUNT_MAX];
long c_count = 0;

struct ModelList{
	char* name;
	long int color, t_cr;
	float x,y,z, r_x,r_y,r_z;
	unsigned long level;
	long int id_l;
};

ModelList* model_list = 0;
long int m_count;

TinyGLContext ctx1;
float angle_x = 135.0, angle_y = 0.0, angle_z = 0.0, delt_size = 3.0,
	scale_o = 0.1, trans_z = 0.0;
double rat_h = 1.0;
bool mouse_drag = false;
short mouse_x, mouse_y;
float angle_dwm, //~ wnd_w/180 - прибавление углов поворота сцены при вращении мышей
	angle_dhm;   //~ wnd_h/180

float light_position[] = {-30.0, 80.0, -50.0, 1.0}; //Расположение источника [0][1][2]
	//[3] = (0.0 - бесконечно удаленный источник, 1.0 - источник света на определенном расстоянии)
float light_dir[] = {0.0,0.0,0.0}; //направление лампы

float mat_specular[] = {0.3, 0.3, 0.3, 1.0}; //Цвет блика
float mat_shininess = 3.0; //Размер блика (обратная пропорция)
float white_light[] = {1.0, 1.0, 1.0, 1.0}; //Цвет и интенсивность освещения, генерируемого источником
float lmodel_ambient[] = {0.3, 0.3, 0.3, 1.0}; //Параметры фонового освещения

char str1[] = "Show active level";
check_box check1 = { {16,310,20,4}, 8, 0xffffff, 0x808080, 0xffffff, str1, ch_flag_middle };
scrollbar sb_tcr = { 200,100,19,4, 16, 1, 20,1,0, 0x808080, 0xffffff, 0x0};

void SetLight()
{
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_dir);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

long list_get_id(char* name)
{
	long i;
	long n = strchr(name, '\'')-name;
	if(n) name[n] = 0;
	for(i=0;i<b_count;i++){
		if(!strcmp(name,b_list[i].name)) return b_list[i].id_l;
	}
	return b_list[0].id_l; //not found
}

long color_get_id(char* name)
{
	long i;
	char* buf;
	for(i=0;i<c_count;i++){
		buf = strchr(c_list[i].name, '=');
		if(buf){
			buf[0]=0;
			while(buf>c_list[i].name && buf[-1]==' '){
				buf--; buf[0]=0;
			};
		}
		while(name[0]==' '){
			name++;
		};
		if(!strcmp(name,c_list[i].name)) return i;
	}
	return -1; //not found
}

void draw_3d()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //очистим буфер цвета и глубины
	glPushMatrix();

	if(b_list){
		SetLight();
		glTranslatef(0.0,0.0,0.5);
		glScalef(scale_o*rat_h,-scale_o,scale_o/4.0); // z/2.0
		glTranslatef(0.0, trans_z, 0.0);
		glRotatef(angle_x,1.0,0.0,0.0);
		glRotatef(angle_y,0.0,1.0,0.0);
		glRotatef(angle_z,0.0,0.0,1.0);

		long i;
		if(model_list){
			unsigned long pu_cou=0, pu_lvl=0;
			for(i=0;i<m_count;i++){
				while(model_list[i].level<=pu_lvl && pu_cou){
					pu_cou--;
					pu_lvl--;
					glPopMatrix();
				};
				pu_lvl=model_list[i].level;
				pu_cou++;
				glPushMatrix();
				glTranslatef(model_list[i].x, model_list[i].y, model_list[i].z);
				glRotatef(model_list[i].r_x, 1.0,0.0,0.0);
				glRotatef(model_list[i].r_y, 0.0,1.0,0.0);
				glRotatef(model_list[i].r_z, 0.0,0.0,1.0);
				if((check1.flags&ch_flag_en && model_list[i].t_cr==sb_tcr.position)||
				(!(check1.flags&ch_flag_en) && model_list[i].t_cr<=sb_tcr.position)){
				glColor3ub((model_list[i].color>>16)&255,
					(model_list[i].color>> 8)&255,
					model_list[i].color&255);
				glCallList(model_list[i].id_l);
				}
			}
			while(pu_cou){
				pu_cou--;
				glPopMatrix();
			};
		}
		else{
			glColor3f(1.0, 1.0, 0.0);
			glCallList(b_list[0].id_l);
		}
	}

	glPopMatrix();
}

void compile_list(BlockList *list){
	long int i;
	list->id_l = glGenLists(1);
	if(list->id_l<1) return; //not found
	glNewList(list->id_l, GL_COMPILE);
		glBegin(GL_TRIANGLES);
		for(i=0;i<list->p_cou;i++){
			glNormal3fv((float*)(list->norm_d+i*3));
			glVertex3fv((float*)(list->vert_d+i*3));
		}
		glEnd();
	glEndList();
}

bool init_block(){
	FileInfoBlock* file;
	unsigned long int k;

	k = strlen(CurrentDirectoryPath);
	while(CurrentDirectoryPath[k] != '\\' && CurrentDirectoryPath[k] != '/' && k) {k--;};
	memcpy(library_path,CurrentDirectoryPath,k);
	if(library_path[k-1] != '/'){
		library_path[k] = '/';
		k++;
	}
	strcpy(library_path+k,"block.bin");

	file = FileOpen(library_path);
	if (!file){
		MessageBox("Error open file 'block.bin', file not found");
		return false;
	}
	k = FileGetLength(file);
	if (k > 0){
		if(b_data) delete b_data;
		b_data = new unsigned char[k];

		if (b_data){
			if(FileRead(file, b_data, k) != k){
				delete b_data; b_data = 0;
			}
			else if((long&)b_data[0]==0x4b43504b){ //"KPCK"
				k = (long&)b_data[4];
				f_data = new unsigned char[k];
				unpack(b_data, f_data);
				delete b_data;
				b_data = f_data;
				f_data = 0;
			}
		}
		FileClose(file);
	} 
	else {
		MessageBox("Error open file 'block.bin', file length == 0");
		FileClose(file);
		return false;
	}

	if (b_data){
		unsigned long i=0, n=0;
		b_count=0;
		while((long&)b_data[i] && i<k){
			while(b_data[i]){ i++; };
			i = (i|3)+1;
			i += 4+((long&)b_data[i])*24;
			b_count++;
		};
		b_list = new BlockList[b_count];
		i=0;
		while((long&)b_data[i] && i<k){
			b_list[n].name = (unsigned char*)(b_data+i);
			while(b_data[i]){ i++; };
			i = (i|3)+1;
			b_list[n].p_cou = (long&)b_data[i];
			i += 4;
			b_list[n].vert_d = (float*)(b_data+i);
			i += b_list[n].p_cou*12;
			b_list[n].norm_d = (float*)(b_data+i);
			i += b_list[n].p_cou*12;
			compile_list(&b_list[n]);
			n++;
		};
	}
	else {
		MessageBox("Error open file 'block.bin', can't unpack file");
	}
	return (bool)b_data;
}

bool init_model()
{
	long i, n, max_time=0;
	char *ft = strstr(f_data, "const");
	char *fe; //end ']'
	char *fp; //perv ','

	c_count=0;
	while(ft && c_count<C_COUNT_MAX){
		fp = ft+5;
		while(fp[0]==' ') fp++;
		c_list[c_count].name = fp;
		ft = strchr(ft, '=')+1;
		fe = strchr(ft, ';');
		fe[0] = 0;
		c_list[c_count].color = StrToInt(ft);
		fe[0] = ';';
		c_count++;
		ft = strstr(ft, "const");
	}

	float mz_min=0.0, mz_max=0.0;
	ft = strstr(f_data, "model_list");
	if(ft==0) return false;

	m_count=0;
	fe=strchr(ft, ';');
	if(fe==0) return false;
	do{
		ft=strchr(ft, '[');
		ft=strchr(ft, ']');
		if(ft && ft<fe) m_count++;
	}while(ft && ft<fe);

	if(model_list) delete model_list;
	model_list = new ModelList[m_count];

	ft = strstr(f_data, "model_list");
	ft=strchr(ft, '[')+1;
	for(i=0;i<m_count;i++){
		ft=strchr(ft, '[')+1;
		fe=strchr(ft, ']')+1;
		ft=strchr(ft, '\'')+1;
		model_list[i].name = ft;
		ft=strchr(ft, ',')+1; //color
		
		fp=ft;
		ft=strchr(ft, ',')+1;
		ft[-1]=0;
		n=color_get_id(fp);
		if(n>-1){
			model_list[i].color=c_list[n].color;
		}
		else{
			model_list[i].color=StrToInt(fp);
		}

		fp=ft;
		ft=strchr(ft, ',')+1;
		ft[-1]=0;
		model_list[i].t_cr=StrToInt(fp);
		if(model_list[i].t_cr>max_time) max_time=model_list[i].t_cr;

		fp=ft;
		ft=strchr(ft, ',')+1;
		ft[-1]=0;
		model_list[i].x=StrToDouble(fp);

		fp=ft;
		ft=strchr(ft, ',')+1;
		ft[-1]=0;
		model_list[i].y=StrToDouble(fp);

		fp=ft;
		ft=strchr(ft, ',')+1;
		ft[-1]=0;
		model_list[i].z=StrToDouble(fp);

		fp=ft;
		ft=strchr(ft, ',')+1;
		ft[-1]=0;
		model_list[i].r_x=StrToDouble(fp);

		fp=ft;
		ft=strchr(ft, ',')+1;
		ft[-1]=0;
		model_list[i].r_y=StrToDouble(fp);

		fp=ft;
		ft=strchr(ft, ',')+1;
		if(!ft || fe<ft){
			ft=fe;
			model_list[i].level=0;
			if(mz_min>model_list[i].z) mz_min=model_list[i].z;
			if(mz_max<model_list[i].z) mz_max=model_list[i].z;
		}
		ft[-1]=0;
		model_list[i].r_z=StrToDouble(fp);
		if(ft!=fe){
			fp=ft;
			ft=fe;
			ft[-1]=0;
			model_list[i].level=StrToInt(fp);
		}
		model_list[i].id_l = list_get_id(model_list[i].name);
	}
	trans_z = (mz_max-mz_min)/2.0;
	scale_o = .5/trans_z;
	angle_x = 135.0;
	angle_z = -45.0;
	sb_tcr.max_area = max_time+1;
	sb_tcr.position = max_time;

	return true;
}

void KolibriOnPaint(void);

void __stdcall DrawWindow()
{
	asm{
		push ebx
		mcall SF_REDRAW,SSF_BEGIN_DRAW
	}
	KolibriOnPaint();
	asm{
		mcall SF_REDRAW,SSF_END_DRAW
		pop ebx
	}
}

bool OpenModel(char* f_path)
{
	FileInfoBlock* file;
	unsigned long int k;

	file = FileOpen(f_path);
	if (!file){
		SetWindowCaption("Error open file ...");
		return false;
	}
	k = FileGetLength(file);
	if (k > 0){
		if(f_data) delete f_data;
		f_data = new unsigned char[k];
		if (f_data){
			if (FileRead(file, f_data, k) != k){
				delete f_data; f_data = 0;
			}
			else{
				init_model();
				draw_3d();
				SetWindowCaption(ofd.openfile_path);
				Redraw(1);
			}
		}
	}
	FileClose(file);
	return (bool)f_data;
}

bool KolibriOnStart(TStartData &kos_start, TThreadData /*th*/)
{
	kos_start.Left = 10;
	kos_start.Top = 40;
	kos_start.Width = 640;
	kos_start.Height = 480;
	kos_start.WinData.WindowColor = 0x333333;
	kos_start.WinData.WindowType = 0x33; // 0x34 - fixed, 0x33 - not fixed
	kos_start.WinData.Title = header;

	if(LoadLibrary("proc_lib.obj", library_path, "/sys/lib/proc_lib.obj", &import_proc_lib))
	{
		ofd.procinfo = procinfo;
		ofd.com_area_name = "FFFFFFFF_open_dialog";
		ofd.com_area = 0;
		ofd.opendir_path = plugin_path;
		ofd.dir_default_path = "/sys";
		ofd.start_path = "/sys/File managers/opendial";
		ofd.draw_window = DrawWindow;
		ofd.status = 0;
		ofd.openfile_path = openfile_path;
		ofd.filename_area = filename_area;
		ofd.filter_area = &filter1;
		ofd.x_size = 420;
		ofd.x_start = 10;
		ofd.y_size = 320;
		ofd.y_start = 10;
		OpenDialog_Init(&ofd);
	} else return false;
	if(LoadLibrary("box_lib.obj", library_path, "/sys/lib/box_lib.obj", &import_box_lib))
	{
		check_box_init(&check1);
		sb_tcr.ar_offset=1;
	} else return false;
	if(LoadLibrary("tinygl.obj", library_path, "/sys/lib/tinygl.obj", &import_tinygl))
	{
		long h = kos_start.Height-TOOLBAR_H;
		kosglMakeCurrent(0,TOOLBAR_H,kos_start.Width,h,&ctx1);
		rat_h = h;
		rat_h /= kos_start.Width;
		angle_dwm = kos_start.Width/180.0;
		angle_dhm = h/180.0;
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.2,0.2,0.2,0.0);
		glEnable(GL_NORMALIZE);
		if(init_block()){
			if(CommandLine[0]) OpenModel(CommandLine);
			return true;
		}
	}
	return false;
}

void KolibriOnPaint(void)
{
	kosglSwapBuffers();

	// If button have ID 1, this is close button
	DrawButton(2,0xf0f0f0, 10,4,50,19);
	DrawText(20,10,0,"Open");
	DrawRect(70,7, 24,18, 0x333333);
	DrawText(70,7,(1<<24)|0xffffff,DoubleToStr(sb_tcr.position,0,true));
	sb_tcr.all_redraw=1;
	scrollbar_h_draw(&sb_tcr);
	check_box_draw(&check1);
}

void KolibriOnButton(long id, TThreadData /*th*/)
{
	switch(id){
	case 2:
		ofd.type = 0; // 0 - open
		OpenDialog_Start(&ofd);
		if(ofd.status==1) OpenModel(ofd.openfile_path);
		//break;
	};
}

void KolibriOnKeyPress(TThreadData /*th*/)
{
	long key = GetKey();
	switch(key){
	case 178: //Up
		angle_x+=delt_size;
		draw_3d();
		kosglSwapBuffers();
		break;
	case 177: //Down
		angle_x-=delt_size;
		draw_3d();
		kosglSwapBuffers();
		break;
	case 176: //Left
		angle_z+=delt_size;
		draw_3d();
		kosglSwapBuffers();
		break;
	case 179: //Right
		angle_z-=delt_size;
		draw_3d();
		kosglSwapBuffers();
		//break;
	};
}

void KolibriOnMouse(TThreadData /*th*/)
{
	short m_x_old, m_y_old;

	long f, m = sb_tcr.position;
	f = check1.flags;
	scrollbar_h_mouse(&sb_tcr);
	check_box_mouse(&check1);
	if(sb_tcr.position!=m || check1.flags!=f){		
		draw_3d();
		Invalidate();
		return;
	}

	m = GetMouseButton();
	if(m&1 && mouse_drag){
		//mouse l. but. move
		m_x_old = mouse_x;
		m_y_old = mouse_y;
		GetMousePosPicture(mouse_x, mouse_y);

		//если курсор движется по оси y (вверх или вниз) то поворот делаем вокруг оси x
		angle_x -= (m_y_old - mouse_y) / angle_dwm;

		//если курсор движется по оси x (влево или вправо) то поворот делаем вокруг оси z
		angle_z -= (m_x_old - mouse_x) / angle_dhm;

		draw_3d();
		kosglSwapBuffers();
	}
	if(m&0x10000){
		//mouse l. but. up
		mouse_drag=false;
	}
	if(m&0x100){
		//mouse l. but. press
		GetMousePosPicture(mouse_x, mouse_y);
		if(mouse_x>0 && mouse_y>TOOLBAR_H) mouse_drag=true;
	}

	GetMouseScrollData(m_x_old, m_y_old);
	if(m_y_old<0 && scale_o<0.5){
		scale_o *= 1.414213562;
		draw_3d();
		kosglSwapBuffers();
	}
	else if(m_y_old>0 && scale_o>0.005){
		scale_o /= 1.414213562;
		draw_3d();
		kosglSwapBuffers();
	}
}

void KolibriOnSize(int [], TThreadData /*th*/)
{
	unsigned short int width, height;
	GetClientSize(width, height);
	if(!width || !height) return;
	width--;
	height-=TOOLBAR_H;
	if(width<100) width=100;
	if(height<80) height=80;
	rat_h = (float)height / (float)width;
	angle_dwm = (float)width/180.0;
	angle_dhm = (float)height/180.0;
	glViewport(0, 0, width, height);
	draw_3d();
}

bool KolibriOnClose(TThreadData /*th*/)
{
	if(b_data){
		delete b_data;
		delete b_list;
	}
	if(f_data) delete f_data;
	if(model_list) delete model_list;
	return true;
}
