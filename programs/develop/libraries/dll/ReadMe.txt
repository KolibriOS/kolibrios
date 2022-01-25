
---
History

0.1     + First realised, kernel load dll.obj at runtime as starting point berfore app startup
          dll.obj process app import table, but not depended librarys, after that app gots control in his starting point

0.2     + Introduced new KX header as extension for current format (see decription below)
        + Add KX header processing
        + Improved import table test logic, no reason to kill app for import absence - skip import processing (tnx ProMiNick)
        + Added ReadMe.txt (this doc)

0.2.1   + Branch from dll.inc, now this file is not external. Improved error handling. Now dll.Load return 0 in success only
          Added corrsponding error codes if one of library or entry not found
        + Added error handling with detailed inform user which error occurred through @notify.
          Now application is not crashed if bad format, can't load library or no found entry		


Purpose
 
Automatically libraries loads and linking imports.	  

---
TODO

1) The library format needs to be improved, see intorduction of KX header extension bellow

---
How to use

- in app:
1) In the version field of a header,  (after MENUET0x) you must specify the number 2
2) After existing header add KX header extension as descriprion bellow
3) Specify imported libraries. Currentry format of import table same as in case of using dll.Load
4) Add code, without connecting dll.inc and, accordingly, without calling dll.Load. The heap initialization function (f. 68.11) does not need to be called either.

5) Compile the app and run. If everything is done correctly, then on startup the debug board will display the message "App header version 2"
   If the DLL.OBJ library is missing, a message will be displayed, incl. via @NOTIFY. If you get a page error make sure you have completed    steps 2 and 3

- in lib (obj):
Not supported yet, will be realized later.
1) Field optHeader of COFF header need set in 8+n*4, where n is count of fields after KX header 
2) After COFF header add KX header extension (in general same as in case for app)

---
Descriprion of KX header extension (alpha).

TBD is meaning that this feature to be determined leter, and not supported yet.
By default all offsets and sizes given in bytes, for Flags field offsets and size given in bits. 
Offset 4.x meaning offset 4 bit x
 
Fields between offset 8 and at end of KX header may be added later.

 Offset	Size    Field			Meaning

		Signature:

 0	2	SigMagic		Module identifier with the value "KX"

 2	1	SigRevision		This field should be 0. 
In the future, it can take on the revision value 
					(but can't take values higher than 64)

 3	1	SigArch			This field should be 0. 


 4	2	Flags:

 4.0	2b	F_ImageType		TBD, this field should be 0 		
 
 4.2	1b	F_SectionMode		TBD, this field should be 0

 4.3	1b	F_Const			TBD, this field should be 0

 4.4	1b	F_Data			TBD, this field should be 0
		
 4.5	1b	F_Export		Module has export table, and pointer after header (see below)

 4.6	1b	F_Import		Module has import table

 4.7	1b	F_Reserved		Reserved, this field should be 0

 4.8	1b	F_BoundImport		TBD, this field should be 0
	
 4.9	1b	F_BSS			TBD, this field should be 0

 4.10	1b	F_TLS			TBD, this field should be 0

 1.11	5b	F_Reserved		Reserved, this field should be 0


 6	2	Reserved		Reserved, this field should be 0	
	
	... 	TBD			Fields in this place may be added later

if(F_Export) {
 ?	4	ExportsHeader		Pointer to export header (exists if F_Export=1), KX style export table not supported yet. Currently 						by backward compatibility reason used legacy style of export table, this field pointed to it 			
}

if(F_Import) {
 ?	4	ImportsHeader		Pointer to imports header (exists if F_Import=1), KX style import table not supported yet. Currently 						by backward compatibility reason used legacy style of import table, this field pointed to it
}

	... 	TBD			Fields in this place may be added later

---
EOF