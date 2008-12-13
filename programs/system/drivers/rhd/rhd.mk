rhd.exe: rhd.obj rhd_id.obj dbg.obj pci.obj rhd_crtc.obj rhd_vga.obj rhd_mc.obj rhd_atombios.obj
 *wlink name rhd.exe SYS nt_dll op offset=0 op nod &
op maxe=25 op el op STUB=stub.exe op START=_drvEntry @rhd.lk1


