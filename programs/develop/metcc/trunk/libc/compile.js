var fso=new ActiveXObject("Scripting.FileSystemObject");
var wsh=WScript.CreateObject("WScript.Shell");
var curpath=".";
var gccpath="c:\\program files\\MinGW\\MinGW\\bin\\";
//var gccpath="cmd.exe /c ";
var gccexe="\""+gccpath+"cc1.exe"+"\" ";
var asexe="\""+gccpath+"as.exe"+"\" ";
var objcopyexe="\""+gccpath+"objcopy.exe"+"\" ";
//var gccexe=gccpath+"cc1.exe" ;
//var asexe=gccpath+"as.exe";
var scriptline="CREATE melibc.a\r\n";

curpath=".\\string\\";
compileasm("memmove");
compileasm("memset");
curpath=".\\mesys\\";
compileasm("backgr");
compileasm("button");
compileasm("clock");
compileasm("date");
compileasm("debug_board");
compileasm("delay");
compileasm("dga");
compileasm("draw_bar");
compileasm("draw_image");
compileasm("draw_window");
compileasm("event");
compileasm("exit");
compileasm("file_58");
compileasm("ipc");
compileasm("irq");
compileasm("keyboard");
compileasm("line");
compileasm("midi");
compileasm("pci");
compileasm("pixel");
compileasm("process");
compileasm("screen");
compileasm("sound");
compileasm("thread");
compileasm("window_redraw");
compileasm("write_text");
curpath=".\\mem\\";
compileasm("memalloc");
curpath=".\\mesys\\";
compilec("debug_board_");
curpath=".\\string\\";
compilec("memchr");
compilec("memcmp");
compilec("strcat");
compilec("strchr");
compilec("strcmp");
compilec("strcoll");
compilec("strcpy");
compilec("strcspn");
compilec("strdup");
compilec("strerror");
compilec("strlen");
compilec("strnbrk");
compilec("strncat");
compilec("strncmp");
compilec("strncpy");
compilec("strrchr");
compilec("strspn");
compilec("strstr");
compilec("strtok");
compilec("strxfrm");
curpath=".\\file\\";
compilec("fclose");
compilec("fopen");
compilec("feof");
compilec("fflush");
compilec("fgetc");
compilec("fgetpos");
compilec("fsetpos");
compilec("fputc");
compilec("fread");
compilec("fwrite");
compilec("fseek");
compilec("ftell");
compilec("rewind");
compilec("fprintf");
compilec("fscanf");
compilec("ungetc");
curpath=".\\start\\";
compileasm("start");
//linking
scriptline+="SAVE\r\n";
linko();
function compileasm(filename)
{
  wsh.Run("fasm.exe "+quote(curpath+filename+".asm")+
    " "+quote(curpath+filename+".o"),0,true);
  addo(filename);
}
function compilec(filename)
{
  wsh.Run(gccexe+"-nostdinc -I .\\include -DGNUC " + quote(curpath + filename + ".c")+
    " -o " + quote(curpath + filename + ".s"),0,true);
  wsh.Run(asexe+quote(curpath+filename+".s")+" -o "+quote(curpath+filename+".o"),0,true);
  wsh.Run(objcopyexe+" -O elf32-i386 --remove-leading-char "+quote(curpath+filename+".o"),0,true);
  addo(filename);
}
function addo(filename)
{
  scriptline+="ADDMOD "+curpath+filename+".o\r\n";
}
function linko()
{
  //fso.DeleteFile(".\\melibc.a");
  var file=fso.OpenTextFile("./script.txt",2,true);
  file.Write(scriptline);
  wsh.Run("cmd.exe /c ar.exe -M < ./script.txt",4,true);
}
function quote(name)
{
  return "\""+name+"\"";
}