
#include "kolibrisys.h"
#include "libGUI.h"

char* sys_libGUI_path="/sys/lib/libGUI.obj";

void link_libGUI(char *exp){

	char	name_DestroyControl[]={"DestroyControl"};
	char	name_SendMessage[]={"SendMessage"};
	char	name_Version[]={"Version"};
	char	name_ResizeComponent[]={"ResizeComponent"};
	char	name_MoveComponent[]={"MoveComponent"};
	char	name_ActivateTrapForSpecializedMessage[]={"ActivateTrapForSpecializedMessage"};
	char	name_CraeteButton[]={"CraeteButton"};
	char	name_CraeteScroller[]={"CraeteScroller"};
	char	name_CraeteBookmark[]={"CraeteBookmark"};
	char	name_CraeteImage[]={"CraeteImage"};
	char	name_CraeteText[]={"CraeteText"};
	char	name_CraeteNumber[]={"CraeteNumber"};
	char	name_CraeteCheckbox[]={"CraeteCheckbox"};
	char	name_CraeteEditbox[]={"CraeteEditbox"};
	char	name_CraeteProgressbar[]={"CraeteProgressbar"};

        DestroyControl=(void stdcall (*)(void *control))
		_ksys_cofflib_getproc(exp,name_DestroyControl);
	SendMessage=(void stdcall (*)(struct HEADER *Parend,struct MESSAGE *Message))
		_ksys_cofflib_getproc(exp,name_SendMessage);
	Version=(int stdcall (*)(void))
		_ksys_cofflib_getproc(exp,name_Version);
	ResizeComponent=(void stdcall(*)(void *Control,int new_sizex,int new_sizey))
		_ksys_cofflib_getproc(exp,name_ResizeComponent);
	MoveComponent=(void stdcall(*)(void *Control,int new_x,int new_y))
		_ksys_cofflib_getproc(exp,name_MoveComponent);
	ActivateTrapForSpecializedMessage=(void stdcall(*)(void *Control,int new_x,int new_y))
		_ksys_cofflib_getproc(exp,name_ActivateTrapForSpecializedMessage);
	CraeteButton=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteButton);
	CraeteScroller=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteScroller);
	CraeteBookmark=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteBookmark);
	CraeteImage=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteImage);
	CraeteText=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteText);
	CraeteNumber=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteNumber);
	CraeteCheckbox=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteCheckbox);
	CraeteEditbox=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteEditbox);
	CraeteProgressbar=(void* stdcall (*)(struct HEADER *Parend,void *Control))
		_ksys_cofflib_getproc(exp,name_CraeteProgressbar);

}

int	Init_libGUI(void)
{
	char	*Export;

	Export=(char *)_ksys_cofflib_load(sys_libGUI_path);
	if (Export==0) return CANNOT_LOAD_LIBGUI;

	link_libGUI(Export);
	return(0);
	
}

