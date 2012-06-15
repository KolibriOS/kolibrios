ml /coff /c /Cx /DMASM6 /DBLD_COFF /DIS_32 vxd.asm
link /vxd /stub:masmstub /def:deffile.def vxd.obj
