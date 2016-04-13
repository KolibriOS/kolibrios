
/* Device flags */
#define DF_NO_MUL    0x00000001
#define DF_NO_JMP    0x00000002	// No JMP, CALL
#define DF_NO_XREG   0x00000004	// No X register
#define DF_NO_YREG   0x00000008	// No Y register
#define DF_TINY1X    0x00000010	/* AT90S1200, ATtiny10-12  set: No ADIW, SBIW,
				   IJMP, ICALL, LDD, STD, LDS, STS, PUSH, POP */
#define DF_NO_LPM    0x00000020	// No LPM instruction
#define DF_NO_LPM_X  0x00000040 // No LPM Rd,Z or LPM Rd,Z+ instruction
#define DF_NO_ELPM   0x00000080	// No ELPM instruction
#define DF_NO_ELPM_X 0x00000100 // No ELPM Rd,Z or LPM Rd,Z+ instruction
#define DF_NO_SPM    0x00000200 // No SPM instruction
#define DF_NO_ESPM   0x00000400 // No ESPM instruction
#define DF_NO_MOVW   0x00000800 // No MOVW instruction
#define DF_NO_BREAK  0x00001000 // No BREAK instruction
#define DF_NO_EICALL 0x00002000 // No EICALL instruction
#define DF_NO_EIJMP  0x00004000 // No EIJMP instruction

struct device
	{
	char *name;
	int flash_size;
	int ram_start;
	int ram_size;
	int eeprom_size;
	int flag;
	};

/* device.c */
struct device *get_device(struct prog_info *pi,char *name);
int predef_dev(struct prog_info *pi);
void list_devices();
