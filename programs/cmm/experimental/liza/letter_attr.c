char from[256];
char to[256];
char date[256];
char subj[256];
dword mdata;

struct letter_attr
{
   char adress[sizeof(to)];
   char subject[sizeof(subj)];
   byte direction;
   int size;
   void CreateArray();
   void SetSizes();
   void SetAtrFromCurr(int N);
   int  GetSize(int N);
   char GetDirection(int N);
   dword GetSubject(int N);
   dword GetAdress(int N);
};
letter_attr atr;
dword mails_db;



void letter_attr::CreateArray()
{
   free(mails_db);
   mails_db = malloc( mail_list.count * sizeof(atr) );
}

void letter_attr::SetSizes()
{
   int i;
   for (i=1; i < mail_list.count; i++)
   {
      ESDWORD[sizeof(atr)*i+#mails_db+#atr.size-#atr] = GetLetterSize_(i);
      ESDWORD[sizeof(atr)*i+#mails_db+#atr.subject-#atr] = ' ';
      ESDWORD[sizeof(atr)*i+#mails_db+#atr.subject-#atr+1] = '\0';
   }
}

void letter_attr::SetAtrFromCurr(int N)
{
   byte mail_direction=0;
   if (strstri(#to, #email_text))
   {
      mail_direction = 1;
      strcpy(sizeof(atr)*N+#mails_db+#atr.adress-#atr, #from);
   }
   if (strstri(#from, #email_text))
   {
      mail_direction = 2;
      strcpy(sizeof(atr)*N+#mails_db+#atr.adress-#atr, #to);
   }
   ESBYTE[sizeof(atr)*N+#mails_db+#atr.direction-#atr] = mail_direction;
   strcpy(sizeof(atr)*N+#mails_db+#atr.subject-#atr, #subj);
}

int letter_attr::GetSize(int N) { return ESDWORD[sizeof(atr)*N+#mails_db+#atr.size-#atr]; }
char letter_attr::GetDirection(int N) { return ESBYTE[sizeof(atr)*N+#mails_db+#atr.direction-#atr]; }
dword letter_attr::GetSubject(int N) { return sizeof(atr)*N+#mails_db+#atr.subject-#atr; }
dword letter_attr::GetAdress(int N) { return sizeof(atr)*N+#mails_db+#atr.adress-#atr; }