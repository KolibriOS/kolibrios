ForReading = 1
ForWriting = 2 
ForAppending = 8 
var fso = new ActiveXObject("Scripting.FileSystemObject");
var wsh = new ActiveXObject("WScript.Shell");
_DEBUG = true;
function _debug(mes)
{
	if(_DEBUG != true) return mes;
	try{var file = fso.OpenTextFile("debug_info.txt", ForAppending);}
	catch(e){var file = fso.CreateTextFile("debug_info.txt", true);}
	file.Write(mes);
	file.close();
	return mes;
}
function alert(mes){WScript.Echo(mes);return mes}

function debug(obj){for(key in obj)alert('['+key+']="'+obj[key]+'"')}
function getFileContent(filename){
	var file = fso.OpenTextFile(filename, ForReading);
	var content = file.ReadAll();
	file.close();
	return content;
}
function include(filename){
	eval(getFileContent(filename));
}
function Project(filename){
	var text = getFileContent(filename);
	eval("var config = {"+text+"};")
	for(key in config) this[key] = config[key];
	if(!this.files) this.files = [];
	function getFileExt(name){
		var i = name.lastIndexOf(".");
		return (i==-1)? "" : name.substr(i);
	}
	this.getFileList = function(folder){
		var f = fso.GetFolder(folder);
		var fc = new Enumerator(f.SubFolders);
		for (; !fc.atEnd(); fc.moveNext())
		{
			var name = fc.item()+"";
			if(name[0]!='.' && !this.isIgnored(name))
				this.getFileList(name);
		}
		delete fc;
		fc = new Enumerator(f.Files);
		for (; !fc.atEnd(); fc.moveNext())
		{
			var name = fc.item()+"";
			if(name[0]!='.' && !this.isIgnored(name))
				this.files.push(name);
		}
	}
	this.clean = function(){
		var fl = new Enumerator(this.files);
		var fo;
		for (; !fl.atEnd(); fl.moveNext()){
			var file = fl.item()
			switch(getFileExt(file)){
			case ".o":
			case ".s":
				fo = fso.GetFile(file);
				fo.Delete();
				break;
			}
		}
		delete fl;
	}
	var objList = [];
	this.compile_asm = function(filename){
		var objname = filename.replace(/.\w{1,3}$/,".o");
 		objList.push(objname);
		if(fso.FileExists(objname)) return;
		wsh.Run(_debug('"'+this.fasm+'" "'+filename+'" "'+objname+'"\n'),0,true);
	}
	this.compile_c = function(filename){
		var objname = filename.replace(/.\w{1,3}$/,".o");
 		objList.push(objname);
		if(fso.FileExists(objname)) return;
		var asmname = filename.replace(/.\w{1,3}$/,".s");
		var command = "";
		if(!fso.FileExists(asmname)){
			command = '"'+this.gccpath +"\\"+ this.gccexe + "\" -nostdinc";
			if(this.include) command += " -I .\\include";
			command +=" -DGNUC" +' "'+filename + '" -o "' + asmname + '"\n';
			wsh.Run(_debug("cmd.exe /c "+command), 0, true);
		}
		command = '"'+this.gccpath +"\\"+ this.asexe +'" "'+ asmname +'" -o "'+ objname +'"\n';
		wsh.Run(_debug("cmd.exe /c "+command), 0, true);
		command = '"'+this.gccpath +"\\"+ this.objcopyexe +'" -O elf32-i386 --remove-leading-char "'+ objname +'"\n';
		wsh.Run(_debug("cmd.exe /c "+command), 0, true);
	}
	this.build = function(){
		var fl = new Enumerator(this.files);
		for (; !fl.atEnd(); fl.moveNext()){
			var file = fl.item()
			switch(getFileExt(file)){
				case ".c": this.compile_c(file);break;
				case ".asm": this.compile_asm(file);break;
				case ".o": objList.push(file);break;
			}
		}
		delete fl;
		fl = new Enumerator(objList);
		
/*		var file = fso.CreateTextFile("OBJLIST.TXT", true);
		file.Write("CREATE "+this.dstpath+'\\'+this.name+".a\r\n");
		for (; !fl.atEnd(); fl.moveNext()){file.Write("ADDMOD "+fl.item()+"\r\n");}
		file.Write("SAVE\r\t");
		file.Close();
		wsh.Run(this.gccpath+"\\ar.exe -M < OBJLIST.TXT", 0, true);*/
		
		var ar = wsh.Exec(_debug(this.gccpath+"\\ar.exe -M\n"))
		ar.StdIn.Write(_debug("CREATE "+this.dstpath+'\\'+this.name+".a\r\n"));
		for (; !fl.atEnd(); fl.moveNext()){ar.StdIn.Write(_debug("ADDMOD "+fl.item()+"\r\n"));}
		ar.StdIn.Write(_debug("SAVE\r\n"));
	}
	this.rebuild = function(){
		this.clean();
		this.build();
	}
	this.isIgnored = function(value){
		for(var i=0; i<this.ignored.length; i++)
			if (this.ignored[i]==value||this.ignored[i]==getFileExt(value)) return true;
		return false;
	}
	this.nothing = function(){alert("Hello")}
	this.getFileList(this.srcpath);
}

try{var confFile = WScript.Arguments(1);}catch(e){var confFile = "make.cfg";}
try{var action = WScript.Arguments(0);}catch(e){var action = "build";}

var conf = new Project(confFile);
conf[action]();

if(conf.autoclean && action != "clean"){conf["clean"]();}
alert("Done");