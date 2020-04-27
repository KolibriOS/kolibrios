ID_CLOSE_BTN	  = 1
ID_COMPILE_BTN	  = 2
ID_EXECUTE_BTN	  = 3
ID_EXECDBG_BTN	  = 4
ID_OPENDLG_BTN	  = 5

center       fix false
SYSTEMCOLORS fix sc
PROCESSINFO  fix pinfo

macro get_sys_colors wnd_skin,font_1 {
 mcall 48,3,SYSTEMCOLORS,sizeof.system_colors
 if wnd_skin <> 0
  or [SYSTEMCOLORS+system_colors.work],0x03000000
 end if
 if font_1 <> 0
  or [SYSTEMCOLORS+system_colors.grab_text],0x10000000
 end if
}

macro draw_caption _edx,_esi {
 mov  edx,_edx
 mov  esi,_esi
 call __draw_caption
}

macro mmov reg,a1,a2 {
 mov reg,(a1) shl 16 + (a2)
}

macro madd reg,a1,a2 {
 add reg,(a1) shl 16 + (a2)
}

macro msub reg,a1,a2 {
 sub reg,(a1) shl 16 + (a2)
}

macro jmpe reg,def,[val,lab] {
 forward
  cmp reg,val
  je  lab
 common
  if ~def eq
   jmp def
  end if
}

macro func name {
 if used name
  name:
}

macro endf {
 end if
}

@^ fix macro comment {
^@ fix }
