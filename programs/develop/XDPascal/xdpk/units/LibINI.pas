unit LibINI;

interface

uses
  KolibriOS, Libs;

type
  TIniKeyCallback = procedure(FileName, SectionName, KeyName, KeyValue: PAnsiChar) stdcall;
  TIniSectionCallback = procedure(FileName, SectionName: PAnsiChar) stdcall;

var
  ini_enum_sections: function(FileName: PAnsiChar; Callback: TIniSectionCallback): Integer stdcall;
  ini_enum_keys:     function(FileName, SectionName: PAnsiChar; Callback: TIniKeyCallback): Integer stdcall;
  ini_get_str:	     function(FileName, SectionName, KeyName: PAnsiChar; var Buf; BufLen: Integer; DefValue: PAnsiChar): Integer stdcall;
  ini_get_int:	     function(FileName, SectionName, KeyName: PAnsiChar; DefValue: Integer): Integer stdcall;
  ini_get_color:	   function(FileName, SectionName, KeyName: PAnsiChar; DefValue: Integer): Integer stdcall;
  ini_set_str:	     function(FileName, SectionName, KeyName: PAnsiChar; var Buf; BufLen: Integer): Integer stdcall;
  ini_set_int:	     function(FileName, SectionName, KeyName: PAnsiChar; Value: Integer): Integer stdcall;
  ini_set_color:	   function(FileName, SectionName, KeyName: PAnsiChar; Value: Integer): Integer stdcall;
  ini_get_shortcut:  function(FileName, SectionName, KeyName: PAnsiChar; DefValue: Integer; var Modifiers: Integer): Integer stdcall; 
  ini_del_section:   function(FileName, SectionName: PAnsiChar): Integer stdcall; 

  procedure LibINI_initialization;
  
implementation

var
  hLibINI: Pointer;  
  
procedure LibINI_initialization;
begin
  hLibIni           := LoadLibrary('/sys/lib/libini.obj');
  ini_enum_sections := GetProcAddress(hLibIni, 'ini_enum_sections');
  ini_enum_keys     := GetProcAddress(hLibIni, 'ini_enum_keys');
  ini_get_str 	    := GetProcAddress(hLibINI, 'ini_get_str');	   
  ini_get_int 	    := GetProcAddress(hLibINI, 'ini_get_int');	   
  ini_get_color 	  := GetProcAddress(hLibINI, 'ini_get_color');	 
  ini_set_str 	    := GetProcAddress(hLibINI, 'ini_set_str');	   
  ini_set_int 	    := GetProcAddress(hLibINI, 'ini_set_int');	   
  ini_set_color 	  := GetProcAddress(hLibINI, 'ini_set_color');	 
  ini_get_shortcut  := GetProcAddress(hLibINI, 'ini_get_shortcut');
  ini_del_section   := GetProcAddress(hLibINI, 'ini_del_section'); 
  InitLibrary(GetProcAddress(hLibIni, 'lib_init'));
end;

end.