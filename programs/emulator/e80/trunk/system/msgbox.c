
///=============================

#define MB_OK 0
#define MB_OKCANCEL 1
#define MB_ABORTRETRYIGNORE 2 
#define MB_YESNOCANCEL 3
#define MB_YESNO 4
#define MB_RETRYCANCEL 5

#define IDOK 1
#define IDCANCEL 2
#define IDABORT 3
#define IDRETRY 4
#define IDIGNORE 5
#define IDYES 6
#define IDNO 7

///=============================

#define LANG_EN

#ifdef LANG_RU
char BTN_OK[]={"OK"};
char BTN_CANCEL[]={"Отмена"};
char BTN_ABORT[]={"Прекратить"};
char BTN_RETRY[]={"Повторить"};
char BTN_INGNORE[]={"Игнорировать"};
char BTN_NO[]={"Нет"};
#endif

#ifdef LANG_EN
char BTN_OK[]={"OK"};
char BTN_CANCEL[]={"Cancel"};
char BTN_ABORT[]={"Abort"};
char BTN_RETRY[]={"Retry"};
char BTN_INGNORE[]={"Ignore"};
char BTN_NO[]={"No"};
#endif

///=============================

kol_struct_import *MSG_BOX_IMPORT = NULL;
int (* _stdcall mb_create)(char *m, char* t);

char msg[1024];
char thread[1024];


///=============================

char MessageBox(char *text, char *caption, int type)
{

int i, j;

if (MSG_BOX_IMPORT == NULL)
	{
	MSG_BOX_IMPORT = kol_cofflib_load("/sys/lib/Msgbox.obj");
	if (MSG_BOX_IMPORT == NULL)
		kol_exit();

	mb_create = kol_cofflib_procload (MSG_BOX_IMPORT, "mb_create");
		if (mb_create == NULL)
			kol_exit();

	}

msg[0] = 255;
msg[1] = 0;

for (i = 2, j = 0; ;i++, j++)
	{
	msg[i] = caption[j];
	if (0 == msg[i])
		break;
	}

i++;
msg[i] = 0;

for (j = 0; ;i++, j++)
	{
	msg[i] = text[j];
	if (0 == msg[i])
		break;
	}

i++;
msg[i] = 0;

switch (type)
	{
	case MB_OK:
		for (j = 0; ;i++, j++)
			{
			msg[i] = BTN_OK[j];
			if (0 == msg[i])
				break;
			}
		break;

	case MB_OKCANCEL:
		for (j = 0; ;i++, j++)
			{
			msg[i] = BTN_OK[j];
			if (0 == msg[i])
				break;
			}

		i++;
		msg[i] = 0;

		for (j = 0; ;i++, j++)
			{
			msg[i] = BTN_CANCEL[j];
			if (0 == msg[i])
				break;
			}
		break;

	default:
		break;

	}
i++;
msg[i] = 0;

mb_create(msg, thread+1024);

for (;;)
	{
	if ( (unsigned char) msg[0] != 255 )
		switch (type)
			{
			case MB_OK:
				if (msg[0] == 1)
					return IDOK;
				else
					return 0;
				break;

			case MB_OKCANCEL:
				switch(msg[0])
					{
					case 1:
						return IDOK;
					case 2:
						return IDCANCEL;
					default:
						return 0;
					};
				break;

			default:
				return 0;

			};

	kol_sleep(10);
	}


}

///=============================

