/*****************************************************************************
;*                                                                           *
;*    This structure define format of LE header for OS/2,Windows exe files   *
;*       ----------------------------------------------------------          *
;*                                                                           *
;*    Author Trigub Serge. B&M&T Corp.                                       *
;*           10 January 1993                                                 *
;*                                                                           *
;*****************************************************************************/

enum CPU_Type
{
	i80286       =0x01,
	i80386       =0x02,
	i80486       =0x03,
	i80586       =0x04,
	i860_N10     =0x20,
	i860_N11     =0x21,
	MIPS_Mark_I  =0x40,
	MIPS_Mark_II =0x41,
	MIPS_Mark_III=0x42
};

struct Module_Type_Flags
{
	unsigned long Module_Is_DLL        [1];
	unsigned long Reserved1            [1];
	unsigned long Errors_In_Module     [1];
	unsigned long Reserved2            [1];
	unsigned long Code_Load_Application[1];
	unsigned long Application_Type     [3];

	unsigned long Reserved3            [2];
	unsigned long No_External_FIXUP    [1];
	unsigned long No_Internal_FIXUP    [1];
	unsigned long Protected_Mode_Only  [1];
	unsigned long Global_Initialization[1];
	unsigned long Multipledata         [1];
	unsigned long Singledata           [1];
};

struct LE_Header
{
	unsigned short Signature;	// Signature 'LE' for exe header
	unsigned char Byte_Order;
	unsigned char Word_Order;
	unsigned long Exec_Format_Level;
	unsigned short CPU_Type;
	unsigned short Target_OS;
	unsigned long Module_Version;
	union{
		unsigned long Type_Flags;
		Module_Type_Flags Flags;
	};
	unsigned long Number_Of_Memory_Pages;
	unsigned long Initial_CS;
	unsigned long Initial_EI;
	unsigned long Initial_SS;
	unsigned long Initial_ESP;
	unsigned long Memory_Page_Size;
	unsigned long Bytes_On_Last_Page;
	unsigned long Fixup_Section_Size;
	unsigned long Fixup_Section_Checksum;
	unsigned long Loader_Section_Size;
	unsigned long Loader_Section_CheckSum;
	unsigned long Object_Table_Offset;
	unsigned long Object_Table_Entries;
	unsigned long Object_Page_Map_Table_Offset;
	unsigned long Object_Iterate_Data_Map_Offset;
	unsigned long Resource_Table_Offset;
	unsigned long Resource_Table_Entries;
	unsigned long Resident_Names_Table_Offset;
	unsigned long Entry_Table_Offset;
	unsigned long Module_Directives_Table_Offset;
	unsigned long Module_Directives_Table_Entries;
	unsigned long Fixup_Page_Table_Offset;
	unsigned long Fixup_Record_Table_Offset;
	unsigned long Imported_Module_Names_Table_Offset;
	unsigned long Imported_Modules_Count;
	unsigned long Imported_Procedure_Name_Table_Offset;
	unsigned long Per_page_Checksum_Table_Offset;
	unsigned long Data_Pages_Offset;
	unsigned long Preload_Page_Count;
	unsigned long Nonresident_Names_Table_Offset;
	unsigned long Nonresident_Names_Table_Length;
	unsigned long Nonresident_Names_Table_Checksum;
	unsigned long Automatic_Data_Object;
	unsigned long Debug_Information_Offset;
	unsigned long Debug_Information_Length;
	unsigned long Preload_Instance_Pages_Number;
	unsigned long Demand_Instance_Pages_Number;
	unsigned long Extra_Heap_Allocation;
	unsigned long Unknown[1];
};

struct OBJ_FLAGS
{
	unsigned long I_O_Privilage_Level  [1];
	unsigned long Conforming_Segment   [1];
	unsigned long BIG_Segment          [1];
	unsigned long Alias_16_16          [1];
	unsigned long Reserved             [1];
	unsigned long Resident_Long_Locable[1];
	unsigned long Segment_Type         [2];
	unsigned long Segment_Invalid      [1];
	unsigned long Segment_Preloaded    [1];
	unsigned long Segment_Shared       [1];
	unsigned long Segment_Discardable  [1];
	unsigned long Segment_Resource     [1];
	unsigned long Segment_Executable   [1];
	unsigned long Segment_Writable     [1];
	unsigned long Segment_Readable     [1];
};

struct Object_Table
{
	unsigned long Virtual_Segment_Size;
	unsigned long Relocation_Base_Address;
	union {
		unsigned long ObjTableFlags;
		OBJ_FLAGS FLAGS;
	};
	unsigned long Page_MAP_Index;
	unsigned long Page_MAP_Entries;
	unsigned long Reserved;
};


enum {
	Segment_Type_Normal,
	Segment_Zero_Filled,
	Segment_Resident,
	Segment_Resident_contiguous
};

struct PM_FLAGS
{
	unsigned char Page_Type:2;
	unsigned char Reserved :6;
	unsigned char End_Page :2;
};

struct Page_Map_Table
{
	unsigned short High_Page_Number;
	unsigned char Low_Page_Number;
//	union{
//		PM_FLAGS SFLAGS;
		unsigned char FLAGS;
//	};
};

enum{//LE_PM_FLG_Page_Type_Enum        ENUM    {
	Legal_Page      =0,
	Iterated_Page   =1,
	Invalid_Page    =2,
	Zero_Filled_Page=3
};

struct Entry_Table
{
	unsigned char Number_of_Entries;
	unsigned char Bungle_Flags;
	unsigned short Object_Index;
//LE_Entry_First_Entry            equ     $
};

struct Entry
{
	unsigned char Entry_Flags;
	union{
		unsigned short Word_Offset;
		unsigned long Dword_Offset;
	};
};

struct Entry_Bungle_Flags
{
	unsigned char Bits_Entry :1;
	unsigned char Valid_Entry:1;
};

struct Fixup_Record_Table
{
	unsigned char Relocation_Address_Type;
	unsigned char Relocation_Type;
	unsigned short Relocation_Page_Offset;
	unsigned char Segment_or_Module_Index;
	unsigned short Offset_Or_Ordinal_Value;
};

struct Rel_Addr_Type
{
	unsigned char Repeat_Offset       :1;
	unsigned char Target_OFFSET_Absent:1;
	unsigned char Rel_Addr_Type       [4];
};

enum// LE_Relocation_Address_Type_ENUM
{
RA_Low_Byte           =0,
RA_16_bits_selector   =2,
RA_32_bits_Far_Pointer=3,
RA_16_bits_Offset     =5,
RA_48_bits_Far_Pointer=6,
RA_32_bits_Offset     =7,
RA_32_bits_EIP_Rel    =8
};

struct Reloc_Type
{
	unsigned char Ordinal_Byte    :1;
	unsigned char Reserv1         :1;
	unsigned char ABS_Dword       :1;
	unsigned char Target_Offset_32:1;
	unsigned char Reserv2         :1;
	unsigned char ADDITIVE_Type   :1;
	unsigned char Reloc_Type      [2];
};

enum //LE_Relocation_Type_ENUM
{
	Internal_Reference=0,
	Imported_Ordinal  =1,
	Imported_Name     =2,
	OS_FIXUP          =3
};

