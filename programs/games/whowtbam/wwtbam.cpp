// �������� ���� "��� ����� ���� �����������?" ��� ���������
// by ������ ���������� (Dron2004)

#include <kosSyst.h>
#include <kosFile.h>
#include <func.h>

char sVersion[] = "����� 0.2";

int status=0;

bool needcleanup = false; //������ ����, ��� ���� ���� ������... ���������� ��� ������������� ������
int questioncount = 0; //����� ��������
int currentquestion = 0; //����� �������� ������� (1, 2, ..., 15)

int askedquestions[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

char friendsAdvice[1]={' '};

char summs[][16]={"0","100","200","300","500","1000","2000","4000","8000","16000","32000","64000","125000","250000","500000","1000000"};
char * question;
char * answerA;
char * answerB;
char * answerC;
char * answerD;
Byte correctanswer=0x00;

int questionlength=0;
int answerAlength=0;
int answerBlength=0;
int answerClength=0;
int answerDlength=0;

bool na50available = true;
bool callfriendavailable = true;
bool zalavailable = true;

int zalA=0;
int zalB=0;
int zalC=0;
int zalD=0;



bool drawA = true;
bool drawB = true;
bool drawC = true;
bool drawD = true;


char * tempquestion;
char * tempanswerA;
char * tempanswerB;
char * tempanswerC;
char * tempanswerD;
Byte tempcorrectanswer=0x00;
int tempquestionlength=0;
int tempanswerAlength=0;
int tempanswerBlength=0;
int tempanswerClength=0;
int tempanswerDlength=0;


const char header[]="�� ��� ���� ��������஬";
void app_halt();



char * filepathname; //���������� ���� � ��� ����� � ����� ��������
void getFilePathName(){
	int lastslashindex=0;
	char ourfilename[]="appdata.dat";
	int tmpcnt=0;

	for (tmpcnt=0;tmpcnt<strlen(kosExePath);tmpcnt++){
		if (kosExePath[tmpcnt]=='/'){lastslashindex=tmpcnt;}
	}
	filepathname = new char [lastslashindex+strlen(ourfilename)+1];

	for (tmpcnt=0; tmpcnt<=lastslashindex; tmpcnt++){
		filepathname[tmpcnt]=kosExePath[tmpcnt];
	}
	for (tmpcnt=0; tmpcnt<strlen(ourfilename); tmpcnt++){
		filepathname[tmpcnt+lastslashindex+1]=ourfilename[tmpcnt];
	}

}

void prepareFileData() { //��������������� ��������
	Byte tmpbyte[1]={0x00};
   CKosFile basefile(filepathname);

   while (tmpbyte[0]!=0x14){
		basefile.Read (tmpbyte,1);
		if (tmpbyte[0]==0x10) {questioncount++;};
   }
  // basefile
}

void loadquestion(){

	//������� ������� ��������� ��������� ���������� ������ �������
//	rtlSrand(kos_GetSystemClock() / 100000);

	int qcodee;

	int skipsleft;

regenerate:
	qcodee=(rtlRand()%questioncount)+1;

	Byte inputbyte[1]={0x00};

	//������ ������ - ��������� ��������� � ������� ����� �����
	tempquestionlength = 0;
	tempanswerAlength = 0;
	tempanswerBlength = 0;
	tempanswerClength = 0;
	tempanswerDlength = 0;


	skipsleft=qcodee;
	CKosFile basefile(filepathname);
	//����� �� �����, ������ ���������� ��� ������
	while (skipsleft>0){
		basefile.Read (inputbyte,1);
		if (inputbyte[0]==0x10){
			skipsleft--;
		}
		inputbyte[0]=0x00;
	}

	//�������� ���������
	basefile.Read (inputbyte,1);

	// ��� �����, ����� ��������� ����������� ������� ��������������� ������ �����������
	// � ���� ������� (�� ����� ����� �� ������; ������ �� 1000000 ������ ���� ���������,
	// ��� ������ �� 100 ������ :-)))

	if (inputbyte[0]==0x05) //˸���� ������
	{
		if (currentquestion > 5){ //˸���� ������� - ������� �� 1 �� 5
			goto regenerate; //���� ��� ��� 6 ������ � ����� - ���� ������ ������
		}
	}
	if (inputbyte[0]==0x06) //������� ������
	{
		if ((currentquestion < 6)||(currentquestion > 10)){ //������� ������� - ������� �� 6 �� 10
			goto regenerate;
		}
	}
	if (inputbyte[0]==0x07) //������� ������
	{
		if (currentquestion < 11){ //������� ������� - ������� �� 11 �� 15
			goto regenerate;
		}
	}


	for (int counter=0; counter <currentquestion; counter++){
		if (askedquestions[counter]==qcodee){goto regenerate;}
	}


	askedquestions[currentquestion-1]=qcodee;


	inputbyte[0]=0x00;
	//�������, ������� �������� � �������
	tempquestionlength = 0;
	tempanswerAlength = 0;
	tempanswerBlength = 0;
	tempanswerClength = 0;
	tempanswerDlength = 0;
	while (inputbyte[0]!=0x01){
		basefile.Read (inputbyte,1);
		tempquestionlength++;
	}
	while (inputbyte[0]!=0x02){
		basefile.Read (inputbyte,1);
		tempanswerAlength++;
	}
	while (inputbyte[0]!=0x03){
		basefile.Read (inputbyte,1);
		tempanswerBlength++;
	}
	while (inputbyte[0]!=0x04){
		basefile.Read (inputbyte,1);
		tempanswerClength++;
	}
	while (inputbyte[0]!=0x08){
		basefile.Read (inputbyte,1);
		tempanswerDlength++;
	}
	//������ ������ ��������!!!!

	if (needcleanup==true){
		delete question;
		delete answerA;
		delete answerB;
		delete answerC;
		delete answerD;
	}
	needcleanup=true;

	tempquestion = new char[tempquestionlength+1];
	tempanswerA = new char[tempanswerAlength+1];
	tempanswerB = new char[tempanswerBlength+1];
	tempanswerC = new char[tempanswerClength+1];
	tempanswerD = new char[tempanswerDlength+1];



	// ������ ������: ��������� � ������ ������ � �������� ������
	CKosFile basefile2(filepathname);
	inputbyte[0]=0x00;
	skipsleft=qcodee;

	//����� �� �����, ������ ���������� ��� ������
	while (skipsleft>0){
		basefile2.Read (inputbyte,1);
		if (inputbyte[0]==0x10){
			skipsleft--;
		}
		inputbyte[0]=0x00;
	}

	basefile2.Read (inputbyte,1); // ��� - ��������� �������. �� � ��� ���������.

	//������ ������
	int currentbyte=0;
	while (inputbyte[0]!=0x01){
		basefile2.Read (inputbyte,1);
		if (inputbyte[0]!=0x01){tempquestion[currentbyte]=inputbyte[0];}
		currentbyte++;
	}
	tempquestion[currentbyte]='\n';

	//������ ����� A
	currentbyte=0;
	while (inputbyte[0]!=0x02){
		basefile2.Read (inputbyte,1);
		if (inputbyte[0]!=0x02){tempanswerA[currentbyte]=inputbyte[0];}
		currentbyte++;
	}
	tempanswerA[currentbyte]='\n';

	//������ ����� B
	currentbyte=0;
	while (inputbyte[0]!=0x03){
		basefile2.Read (inputbyte,1);
		if (inputbyte[0]!=0x03){tempanswerB[currentbyte]=inputbyte[0];}
		currentbyte++;
	}
	tempanswerB[currentbyte]='\n';

	//������ ����� C
	currentbyte=0;
	while (inputbyte[0]!=0x04){
		basefile2.Read (inputbyte,1);
		if (inputbyte[0]!=0x04){tempanswerC[currentbyte]=inputbyte[0];}
		currentbyte++;
	}
	tempanswerC[currentbyte]='\n';

	//������ ����� D
	currentbyte=0;
	while (inputbyte[0]!=0x08){
		basefile2.Read (inputbyte,1);
		if (inputbyte[0]!=0x08){tempanswerD[currentbyte]=inputbyte[0];}
		currentbyte++;
	}
	tempanswerD[currentbyte]='\n';

	basefile2.Read (inputbyte,1); // ���-���������� �����
	tempcorrectanswer=inputbyte[0];
	// �Ѩ!!!!!! ������!!!! ���!

	//�������. ������ ���� ������������ �������.
	questionlength = 0;
	answerAlength = 0;
	answerBlength = 0;
	answerClength = 0;
	answerDlength = 0;
	//��� ������ ������� ��� ���������
	questionlength=tempquestionlength;
	question = new char[questionlength];
	for (int cd=0; cd<questionlength; cd++){
		question[cd]=tempquestion[cd];
	}


	//������ ������
	bool answerAfree = true;
	bool answerBfree = true;
	bool answerCfree = true;
	bool answerDfree = true;

	int tmpvalue=0;

	//������ ������
regenA:
	tmpvalue = (rtlRand() % 4) +1;
	if (tmpvalue==1){
		if (answerAfree==true){
			answerAfree=false;
			answerAlength=tempanswerAlength;
			answerA= new char [answerAlength];

			for (int c=0; c<answerAlength; c++){
				answerA[c]=tempanswerA[c];
			}
			if (tempcorrectanswer==0x01){correctanswer=0x01;}
		}
		else
		{
			goto regenA;
		}
	}
	if (tmpvalue==2){
		if (answerBfree==true){
			answerBfree=false;
			answerBlength=tempanswerAlength;
			answerB= new char [answerBlength];

			for (int c=0; c<answerBlength; c++){
				answerB[c]=tempanswerA[c];
			}
			if (tempcorrectanswer==0x01){correctanswer=0x02;}
		}
		else
		{
			goto regenA;
		}
	}
	if (tmpvalue==3){
		if (answerCfree==true){
			answerCfree=false;
			answerClength=tempanswerAlength;
			answerC= new char [answerClength];

			for (int c=0; c<answerClength; c++){
				answerC[c]=tempanswerA[c];
			}
			if (tempcorrectanswer==0x01){correctanswer=0x03;}
		}
		else
		{
			goto regenA;
		}
	}
	if (tmpvalue==4){
		if (answerDfree==true){
			answerDfree=false;
			answerDlength=tempanswerAlength;
			answerD= new char [answerDlength];

			for (int c=0; c<answerDlength; c++){
				answerD[c]=tempanswerA[c];
			}
			if (tempcorrectanswer==0x01){correctanswer=0x04;}
		}
		else
		{
			goto regenA;
		}
	}

//������ ������
regenB:
	tmpvalue = (rtlRand() % 4) +1;
	if (tmpvalue==1){
		if (answerAfree==true){
			answerAfree=false;
			answerAlength=tempanswerBlength;
			answerA= new char [answerAlength];

			for (int c=0; c<answerAlength; c++){
				answerA[c]=tempanswerB[c];
			}
			if (tempcorrectanswer==0x02){correctanswer=0x01;}
		}
		else
		{
			goto regenB;
		}
	}
	if (tmpvalue==2){
		if (answerBfree==true){
			answerBfree=false;
			answerBlength=tempanswerBlength;
			answerB= new char [answerBlength];

			for (int c=0; c<answerBlength; c++){
				answerB[c]=tempanswerB[c];
			}
			if (tempcorrectanswer==0x02){correctanswer=0x02;}
		}
		else
		{
			goto regenB;
		}
	}
	if (tmpvalue==3){
		if (answerCfree==true){
			answerCfree=false;
			answerClength=tempanswerBlength;
			answerC= new char [answerClength];

			for (int c=0; c<answerClength; c++){
				answerC[c]=tempanswerB[c];
			}
			if (tempcorrectanswer==0x02){correctanswer=0x03;}
		}
		else
		{
			goto regenB;
		}
	}
	if (tmpvalue==4){
		if (answerDfree==true){
			answerDfree=false;
			answerDlength=tempanswerBlength;
			answerD= new char [answerDlength];

			for (int c=0; c<answerDlength; c++){
				answerD[c]=tempanswerB[c];
			}
			if (tempcorrectanswer==0x02){correctanswer=0x04;}
		}
		else
		{
			goto regenB;
		}
	}

	//������ ������
regenC:
		tmpvalue = (rtlRand() % 4) +1;
	if (tmpvalue==1){
		if (answerAfree==true){
			answerAfree=false;
			answerAlength=tempanswerClength;
			answerA= new char [answerAlength];

			for (int c=0; c<answerAlength; c++){
				answerA[c]=tempanswerC[c];
			}
			if (tempcorrectanswer==0x03){correctanswer=0x01;}
		}
		else
		{
			goto regenC;
		}
	}
	if (tmpvalue==2){
		if (answerBfree==true){
			answerBfree=false;
			answerBlength=tempanswerClength;
			answerB= new char [answerBlength];

			for (int c=0; c<answerBlength; c++){
				answerB[c]=tempanswerC[c];
			}
			if (tempcorrectanswer==0x03){correctanswer=0x02;}
		}
		else
		{
			goto regenC;
		}
	}
	if (tmpvalue==3){
		if (answerCfree==true){
			answerCfree=false;
			answerClength=tempanswerClength;
			answerC= new char [answerClength];

			for (int c=0; c<answerClength; c++){
				answerC[c]=tempanswerC[c];
			}
			if (tempcorrectanswer==0x03){correctanswer=0x03;}
		}
		else
		{
			goto regenC;
		}
	}
	if (tmpvalue==4){
		if (answerDfree==true){
			answerDfree=false;
			answerDlength=tempanswerClength;
			answerD= new char [answerDlength];

			for (int c=0; c<answerDlength; c++){
				answerD[c]=tempanswerC[c];
			}
			if (tempcorrectanswer==0x03){correctanswer=0x04;}
		}
		else
		{
			goto regenC;
		}
	}

	//�������� ������
regenD:
	tmpvalue = (rtlRand() % 4) +1;
	if (tmpvalue==1){
		if (answerAfree==true){
			answerAfree=false;
			answerAlength=tempanswerDlength;
			answerA= new char [answerAlength];

			for (int c=0; c<answerAlength; c++){
				answerA[c]=tempanswerD[c];
			}
			if (tempcorrectanswer==0x04){correctanswer=0x01;}
		}
		else
		{
			goto regenD;
		}
	}
	if (tmpvalue==2){
		if (answerBfree==true){
			answerBfree=false;
			answerBlength=tempanswerDlength;
			answerB= new char [answerBlength];

			for (int c=0; c<answerBlength; c++){
				answerB[c]=tempanswerD[c];
			}
			if (tempcorrectanswer==0x04){correctanswer=0x02;}
		}
		else
		{
			goto regenD;
		}
	}
	if (tmpvalue==3){
		if (answerCfree==true){
			answerCfree=false;
			answerClength=tempanswerDlength;
			answerC= new char [answerClength];

			for (int c=0; c<answerClength; c++){
				answerC[c]=tempanswerD[c];
			}
			if (tempcorrectanswer==0x04){correctanswer=0x03;}
		}
		else
		{
			goto regenD;
		}
	}
	if (tmpvalue==4){
		if (answerDfree==true){
			answerDfree=false;
			answerDlength=tempanswerDlength;
			answerD= new char [answerDlength];

			for (int c=0; c<answerDlength; c++){
				answerD[c]=tempanswerD[c];
			}
			if (tempcorrectanswer==0x04){correctanswer=0x04;}
		}
		else
		{
			goto regenD;
		}
	}

	delete tempquestion;
	delete tempanswerA;
	delete tempanswerB;
	delete tempanswerC;
	delete tempanswerD;
}



void draw_window(void){ //������ ����
	sProcessInfo sPI;

	kos_WindowRedrawStatus(1);
	kos_DefineAndDrawWindow(10,10,619,179+kos_GetSkinHeight(),0x74, 0xDDDDFF, 0,0, (Dword)header);
	kos_WindowRedrawStatus(2);

	kos_ProcessInfo( &sPI );
	if (sPI.rawData[70]&0x04) return; //������ �� ������ ���� ���� ��������� � ���������


	if (status==0){ //����
		kos_DrawBar(0,0,610,175,0xFFFFBB);
                kos_WriteTextToWindow (10,10,0x80,0x000000, "�� ��� ���� ��������஬?", 3);

		kos_WriteTextToWindow (10,25,0x80,0x000000, sVersion, 3);

                kos_WriteTextToWindow (10,70,0x80,0x770000, "<ENTER> - ����� ����", 0);
                kos_WriteTextToWindow (10,85,0x80,0x770000, "<ESC> - ��室", 0);

                kos_WriteTextToWindow (10,150,0x80,0x000000, "(C) 2008 ���३ ��堩����� aka Dron2004", 0);
		//kos_DisplayNumberToWindow (questioncount,3,40,60,0x000000, nbDecimal, false);
	}
	if (status==1){ //����

		kos_DrawBar(0,0,610,175,0xEEEEFF);

		kos_WriteTextToWindow (10,10,0x0,0x000000, question, questionlength-1);

		if (drawA==true){
			kos_WriteTextToWindow (10,40,0x80,0x000000, "A. ", 0);
			kos_WriteTextToWindow (30,40,0x0,0x000000, answerA, answerAlength-1);
		}
		if (drawB==true){
			kos_WriteTextToWindow (10,60,0x80,0x000000, "B. ", 0);
			kos_WriteTextToWindow (30,60,0x0,0x000000, answerB, answerBlength-1);
		}
		if (drawC==true){
			kos_WriteTextToWindow (10,80,0x80,0x000000, "C. ", 0);
			kos_WriteTextToWindow (30,80,0x0,0x000000, answerC, answerClength-1);
		}
		if (drawD==true){
			kos_WriteTextToWindow (10,100,0x80,0x000000, "D. ", 0);
			kos_WriteTextToWindow (30,100,0x0,0x000000, answerD, answerDlength-1);
		}
                if (na50available==true){kos_WriteTextToWindow (30,150,0x80,0x000000, "<7> 50 �� 50", 0);}
                if (callfriendavailable==true){kos_WriteTextToWindow (150,150,0x80,0x000000, "<8> ������ ����", 0);}
                if (zalavailable==true){kos_WriteTextToWindow (280,150,0x80,0x000000, "<9> ���᪠��� ����", 0);}

                if((na50available==false)&&(callfriendavailable==false)&&(zalavailable==false)) {kos_WriteTextToWindow (30,150,0x80,0x000000, "<BACKSPACE> - ������ ���죨 � ��", 0);}

                kos_WriteTextToWindow (430,130,0x80,0x000000, "����� ��", 0);
		kos_WriteTextToWindow (500,130,0x80,0x000000, summs[currentquestion], 0);

                kos_WriteTextToWindow (430,150,0x80,0x000000, "� ���", 0);
		kos_WriteTextToWindow (500,150,0x80,0x000000, summs[currentquestion-1], 0);



	}
	if (status==2){ //���� "��� - ���������� �����"
		kos_DrawBar(0,0,610,175,0xDDFFDD);
                kos_WriteTextToWindow (10,10,0x80,0x000000, "��, �� �ࠢ���� �⢥�!", 0);

                kos_WriteTextToWindow (10,150,0x80,0x000000, "<ENTER> - �த������", 0);
	}
	if (status==3){ //�� �������� �������, ������ �!!!
		kos_DrawBar(0,0,610,175,0x00FF00);
                kos_WriteTextToWindow (10,10,0x80,0x000000, "�� �먣ࠫ� �������!!!", 0);
                kos_WriteTextToWindow (10,150,0x80,0x000000, "<ESC> - ��室", 0);
	}
	if (status==4){ //������ �����
		kos_DrawBar(0,0,610,175,0xAAFFFF);
                kos_WriteTextToWindow (10,10,0x80,0x000000, "��� ᮢ���� ��� �⢥�", 0);
		kos_WriteTextToWindow (165,10,0x80,0x000000, friendsAdvice, 0);
                kos_WriteTextToWindow (10,150,0x80,0x000000, "<ENTER> - �த������", 0);
	}
	if (status==5){ //��������� ����
		kos_DrawBar(0,0,610,175,0xAAFFFF);
                kos_WriteTextToWindow (10,10,0x80,0x000000, "������ �㤨�ਨ ��।������� ⠪:", 0);
		if (drawA==true){
                        kos_WriteTextToWindow (10,30,0x80,0x000000, "�⢥� A:  ", 0);
			kos_DisplayNumberToWindow(zalA,3,60,30,0x000000,nbDecimal,0);
			kos_WriteTextToWindow (80,30,0x80,0x000000, "%", 0);
		}
		if (drawB==true){
                        kos_WriteTextToWindow (10,45,0x80,0x000000, "�⢥� B:  ", 0);
			kos_DisplayNumberToWindow(zalB,3,60,45,0x000000,nbDecimal,0);
			kos_WriteTextToWindow (80,45,0x80,0x000000, "%", 0);
		}
		if (drawC==true){
                        kos_WriteTextToWindow (10,60,0x80,0x000000, "�⢥� C:  ", 0);
			kos_DisplayNumberToWindow(zalC,3,60,60,0x000000,nbDecimal,0);
			kos_WriteTextToWindow (80,60,0x80,0x000000, "%", 0);
		}
		if (drawD==true){
                        kos_WriteTextToWindow (10,75,0x80,0x000000, "�⢥� D:  ", 0);
			kos_DisplayNumberToWindow(zalD,3,60,75,0x000000,nbDecimal,0);
			kos_WriteTextToWindow (80,75,0x80,0x000000, "%", 0);
		}





                kos_WriteTextToWindow (10,150,0x80,0x000000, "<ENTER> - �த������", 0);
	}

	if (status==6){ //�� ������� ������ ;-)
		kos_DrawBar(0,0,610,175,0xBBFFBB);
                kos_WriteTextToWindow (10,10,0x80,0x000000, "�� ���ࠫ� ���죨 � �諨. ��� �먣��� ��⠢��:", 0);
		kos_WriteTextToWindow (10,20,0x80,0x000000, summs[currentquestion-1], 0);
                kos_WriteTextToWindow (10,150,0x80,0x000000, "<ESC> - ��室", 0);
	}
	if (status==-1){ //�� �������� :-(
		kos_DrawBar(0,0,610,175,0xFF8888);
                kos_WriteTextToWindow (10,10,0x80,0x000000, "� ᮦ������, �� �訡����... �ࠢ���� �⢥� -", 0);

		switch (correctanswer){
		case 0x01:
			kos_WriteTextToWindow (10,25,0x80,0x000000, "A. ", 0);
			kos_WriteTextToWindow (30,25,0x0,0x000000, answerA, answerAlength-1);
			break;
		case 0x02:
			kos_WriteTextToWindow (10,25,0x80,0x000000, "B. ", 0);
			kos_WriteTextToWindow (30,25,0x0,0x000000, answerB, answerBlength-1);
			break;
		case 0x03:
			kos_WriteTextToWindow (10,25,0x80,0x000000, "C. ", 0);
			kos_WriteTextToWindow (30,25,0x0,0x000000, answerC, answerClength-1);
			break;
		case 0x04:
			kos_WriteTextToWindow (10,25,0x80,0x000000, "D. ", 0);
			kos_WriteTextToWindow (30,25,0x0,0x000000, answerD, answerDlength-1);
			break;
		}
        kos_WriteTextToWindow (10,50,0x80,0x000000, "� १���� �� �먣ࠫ�:", 0);

	if (currentquestion<6) {kos_WriteTextToWindow (220,50,0x80,0x000000,summs[0], 0);}
	if ((currentquestion>5)&&(currentquestion<11)) {kos_WriteTextToWindow (220,50,0x80,0x000000,summs[5], 0);}
	if (currentquestion>10) {kos_WriteTextToWindow (220,50,0x80,0x000000,summs[10], 0);}




        kos_WriteTextToWindow (10,150,0x80,0x000000, "<ESC> - ��室", 0);
	}

}



void call_friend(){
	int tmpcodee;

	recode5:
	tmpcodee =(rtlRand()%10)+1;
	int tmpbyte;

	if (currentquestion < 6 ){
		if (tmpcodee>3){ //���� �����
			if (correctanswer==0x01) {friendsAdvice[0]='A';}
			if (correctanswer==0x02) {friendsAdvice[0]='B';}
			if (correctanswer==0x03) {friendsAdvice[0]='C';}
			if (correctanswer==0x04) {friendsAdvice[0]='D';}
		}
		else //���� ������� ������
		{

			int tmpbyte2=0;
			recode51:
			int tmpcodee2=(rtlRand()%4)+1;

			switch(tmpcodee2){
			case 1:
				friendsAdvice[0]='A';
				break;
			case 2:
				friendsAdvice[0]='B';
				break;
			case 3:
				friendsAdvice[0]='C';
				break;
			case 4:
				friendsAdvice[0]='D';
				break;
			}
		}

	}

	if ((currentquestion > 5) && (currentquestion<11)){
		if (tmpcodee>5){ //���� �����
			if (correctanswer==0x01) {friendsAdvice[0]='A';}
			if (correctanswer==0x02) {friendsAdvice[0]='B';}
			if (correctanswer==0x03) {friendsAdvice[0]='C';}
			if (correctanswer==0x04) {friendsAdvice[0]='D';}
		}
		else //���� ������� ������
		{

			int tmpbyte2=0;
			recode52:
			int tmpcodee2=(rtlRand()%4)+1;
			switch(tmpcodee2){
			case 1:
				friendsAdvice[0]='A';
				break;
			case 2:
				friendsAdvice[0]='B';
				break;
			case 3:
				friendsAdvice[0]='C';
				break;
			case 4:
				friendsAdvice[0]='D';
				break;
			}
		}

	}

	if (currentquestion > 10){
		if (tmpcodee>7){ //���� �����
			if (correctanswer==0x01) {friendsAdvice[0]='A';}
			if (correctanswer==0x02) {friendsAdvice[0]='B';}
			if (correctanswer==0x03) {friendsAdvice[0]='C';}
			if (correctanswer==0x04) {friendsAdvice[0]='D';}
		}
		else //���� ������� ������
		{

			int tmpbyte2=0;
			recode53:
			int tmpcodee2=(rtlRand()%4)+1;

			switch(tmpcodee2){
			case 1:
				friendsAdvice[0]='A';
				break;
			case 2:
				friendsAdvice[0]='B';
				break;
			case 3:
				friendsAdvice[0]='C';
				break;
			case 4:
				friendsAdvice[0]='D';
				break;
			}
		}

		if ((friendsAdvice[0]=='A')&&(drawA==false)){goto recode5;}
		if ((friendsAdvice[0]=='B')&&(drawB==false)){goto recode5;}
		if ((friendsAdvice[0]=='C')&&(drawC==false)){goto recode5;}
		if ((friendsAdvice[0]=='D')&&(drawD==false)){goto recode5;}
	}






}

void call_zal(){ //��������� ����
	int maxpercent=0;
	for (int tmpc=0; tmpc<(16-currentquestion);tmpc=tmpc+2){
		maxpercent=(rtlRand()%101);
		if (maxpercent>50) {break;}
	}

	if ((drawA==true)&&(drawB==true)&&(drawC==true)&&(drawD==true)){
		switch (correctanswer){
		case 0x01:
		zalA=maxpercent;
		zalB=(rtlRand()%(101-zalA));
		zalC=(rtlRand()%(101-zalA-zalB));
		zalD=100-zalA-zalB-zalC;
		break;

		case 0x02:
		zalB=maxpercent;
		zalA=(rtlRand()%(101-zalB));
		zalC=(rtlRand()%(101-zalA-zalB));
		zalD=100-zalA-zalB-zalC;
		break;

		case 0x03:
		zalC=maxpercent;
		zalB=(rtlRand()%(101-zalC));
		zalA=(rtlRand()%(101-zalC-zalB));
		zalD=100-zalA-zalB-zalC;
		break;

		case 0x04:
		zalD=maxpercent;
		zalB=(rtlRand()%(101-zalD));
		zalC=(rtlRand()%(101-zalD-zalB));
		zalA=100-zalD-zalB-zalC;
		break;
		}
	}
	else
	{
		if ((drawA==true)&&(drawB==true)){
			if (correctanswer==0x01){
				zalA=maxpercent;
				zalB=100-zalA;
			}
			else
			{
				zalB=maxpercent;
				zalA=100-zalB;
			}
		}
		if ((drawA==true)&&(drawC==true)){
			if (correctanswer==0x01){
				zalA=maxpercent;
				zalC=100-zalA;
			}
			else
			{
				zalC=maxpercent;
				zalA=100-zalC;
			}
		}
		if ((drawA==true)&&(drawD==true)){
			if (correctanswer==0x01){
				zalA=maxpercent;
				zalD=100-zalA;
			}
			else
			{
				zalD=maxpercent;
				zalA=100-zalD;
			}
		}

		if ((drawB==true)&&(drawC==true)){
			if (correctanswer==0x02){
				zalB=maxpercent;
				zalC=100-zalB;
			}
			else
			{
				zalC=maxpercent;
				zalB=100-zalC;
			}
		}
		if ((drawB==true)&&(drawD==true)){
			if (correctanswer==0x02){
				zalB=maxpercent;
				zalD=100-zalB;
			}
			else
			{
				zalD=maxpercent;
				zalB=100-zalD;
			}
		}

		if ((drawC==true)&&(drawD==true)){
			if (correctanswer==0x03){
				zalC=maxpercent;
				zalD=100-zalC;
			}
			else
			{
				zalD=maxpercent;
				zalC=100-zalD;
			}
		}

	}
}

void kos_Main(){
	rtlSrand(kos_GetSystemClock() / 10000);
	kos_InitHeap();
	getFilePathName();
	prepareFileData();
	draw_window();
	while (true){

		switch (kos_WaitForEvent()){
		case 1:
			draw_window();
			break;
		case 2:
			Byte keyCode;
			kos_GetKey(keyCode);

			if (status==0){ //����
				if (keyCode==27){
					app_halt();
				}
				if (keyCode==13){
					currentquestion=1;
					status=1;
					loadquestion();
					////// �������� �������!!!!!!! /////////
					draw_window();

				}
			}
			if (status==1){ //����

			if (keyCode==8){
				status=6;
				draw_window();
			}

			if (drawA==true){
			if ((keyCode==49)||(keyCode==97)||(keyCode==65)){
					if (correctanswer==0x01){
						status=2;
					}
					else
					{
						status=-1;
					}
					drawA = true;
					drawB = true;
					drawC = true;
					drawD = true;

					draw_window();
			}
			}

			if (drawB==true){
			if ((keyCode==50)||(keyCode==98)||(keyCode==66)){
					if (correctanswer==0x02){
						status=2;
					}
					else
					{
						status=-1;
					}
					drawA = true;
					drawB = true;
					drawC = true;
					drawD = true;

					draw_window();
				}
			}
			if (drawC==true){
			if ((keyCode==51)||(keyCode==99)||(keyCode==67)){
					if (correctanswer==0x03){
						status=2;
					}
					else
					{
						status=-1;
					}
					drawA = true;
					drawB = true;
					drawC = true;
					drawD = true;

					draw_window();
				}
			}
			if (drawD==true){
			if ((keyCode==52)||(keyCode==100)||(keyCode==68)){
					if (correctanswer==0x04){
						status=2;
					}
					else
					{
						status=-1;
					}
					drawA = true;
					drawB = true;
					drawC = true;
					drawD = true;

					draw_window();
			}
			}

			if (callfriendavailable==true){ //���������� ��������� "������ �����"
				if (keyCode==56){
					callfriendavailable=false;
					status=4;
					call_friend();
					draw_window();
				}
			}

			if (zalavailable==true){ //���������� ��������� ����
				if (keyCode==57){
					zalavailable=false;
					status=5;
					call_zal();
					draw_window();
				}
			}

			if (na50available==true){ //���������� ��������� "50 �� 50"
			if (keyCode==55){

				if (correctanswer==0x01){
					drawA=true;

						int tmpcodee;

						recode1:
						tmpcodee =(rtlRand()%3)+1;

						int tmpbyte;

						switch(tmpcodee){
						case 1:
							drawB=true;
							drawC=false;
							drawD=false;
							break;
						case 2:
							drawB=false;
							drawC=true;
							drawD=false;
						case 3:
							drawB=false;
							drawC=false;
							drawD=true;

						}
				}
				if (correctanswer==0x02){
					drawB=true;

						int tmpcodee;

						recode2:
						tmpcodee =(rtlRand()%3)+1;

						int tmpbyte;


						switch(tmpcodee){
						case 1:
							drawA=true;
							drawC=false;
							drawD=false;
							break;
						case 2:
							drawA=false;
							drawC=true;
							drawD=false;
						case 3:
							drawA=false;
							drawC=false;
							drawD=true;

						}
				}
				if (correctanswer==0x03){
					drawC=true;

						int tmpcodee;

						recode3:
						tmpcodee =(rtlRand()%3)+1;
						int tmpbyte;


						switch(tmpcodee){
						case 1:
							drawB=true;
							drawA=false;
							drawD=false;
							break;
						case 2:
							drawB=false;
							drawA=true;
							drawD=false;
						case 3:
							drawB=false;
							drawA=false;
							drawD=true;

						}
				}
				if (correctanswer==0x04){
					drawA=true;

						int tmpcodee;

						recode4:
						tmpcodee =(rtlRand()%3)+1;

						int tmpbyte;


						switch(tmpcodee){
						case 1:
							drawB=true;
							drawC=false;
							drawA=false;
							break;
						case 2:
							drawB=false;
							drawC=true;
							drawA=false;
						case 3:
							drawB=false;
							drawC=false;
							drawA=true;

						}
				}
				na50available=false;
				draw_window();

			}
			}

			}
			if (status==2){ //���� "��� - ���������� �����!"
				if (keyCode==13){
					if (currentquestion<15){
						currentquestion++;
						status=1;
						loadquestion();
						draw_window();
					}
					else
					{
						status=3;
						draw_window();
					}

				}
			}
			if (status==3){ //�� �������� �������
				if (keyCode==27){
					app_halt();
				}
			}
			if (status==4){ //����� �����
				if (keyCode==13){
					status=1;
					draw_window();
				}
			}
			if (status==5){ //��������� ����
				if (keyCode==13){
					status=1;
					draw_window();
				}
			}
			if (status==6){ //�� ������� ������ ;-)
				if (keyCode==27){
					app_halt();
				}
			}
			if (status==-1){ //�� �������� :-(
				if (keyCode==27){
					app_halt();
				}
			}

			//kos_DrawBar(38,118,50,130,0xBBBBBB);
			//kos_DisplayNumberToWindow (keyCode,3,40,120,0x000000, nbDecimal, false);


			break;
		case 3:
			app_halt();
			break;
		}

	}
}


void app_halt(){
	delete filepathname;

	if (needcleanup==true){
		delete question;
		delete answerA;
		delete answerB;
		delete answerC;
		delete answerD;
	}
	kos_ExitApp();
}
