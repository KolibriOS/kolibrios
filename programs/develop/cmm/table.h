/* some defines for extracting instruction bit fields from bytes */
#define MOD(a)	  (((a)>>6)&3)
#define REG(a)	  (((a)>>3)&7)
#define RM(a)	  ((a)&7)
#define SCALE(a)  (((a)>>6)&3)
#define INDEX(a)  (((a)>>3)&7)
#define BASE(a)   ((a)&7)

typedef union{
  struct{
    unsigned short ofs;
    unsigned short seg;
  }w;
  unsigned long dword;
}WORD32;

/* prototypes */
void ua_str(char *);
unsigned char getbyte(void);
int modrm();
int sib();
void uprintf(char *, ...);
void uputchar(char );
int bytes(char );
void outhex(char , int , int , int , int );
void reg_name(int , char );
void do_sib(int );
void do_modrm(char );
void floating_point(int );
void percent(char , char );
void undata(unsigned ofs,unsigned long len,unsigned int type);

