iglobal
  boot_memdetect    db   'Determining amount of memory',0
  boot_fonts        db   'Fonts loaded',0
  boot_tss          db   'Setting TSSs',0
  boot_cpuid        db   'Reading CPUIDs',0
  boot_devices      db   'Detecting devices',0
  boot_timer        db   'Setting timer',0
  boot_irqs         db   'Reprogramming IRQs',0
  boot_setmouse     db   'Setting mouse',0
  boot_windefs      db   'Setting window defaults',0
  boot_bgr          db   'Calculating background',0
  boot_resirqports  db   'Reserving IRQs & ports',0
  boot_setrports    db   'Setting addresses for IRQs',0
  boot_setostask    db   'Setting OS task',0
  boot_allirqs      db   'Unmasking all IRQs',0
  boot_tsc          db   'Reading TSC',0
  boot_pal_ega      db   'Setting EGA/CGA 320x200 palette',0
  boot_pal_vga      db   'Setting VGA 640x480 palette',0
  boot_mtrr         db   'Setting MTRR',0
  boot_tasking      db   'All set - press ESC to start',0
endg

iglobal
  boot_y dd 10
endg
