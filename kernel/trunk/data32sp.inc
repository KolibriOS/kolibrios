  boot_initirq      db   'Inicializar IRQ',0
  boot_picinit      db   'Inicializar PIC',0
  boot_v86machine   db   'Inicializar sistema V86',0
  boot_inittimer    db   'Inicializar reloj del sistema (IRQ0)',0
  boot_initapic     db   'Prueba inicializar APIC',0
  boot_enableirq    db   'Habilitar interrupciones 2, 6, 13, 14, 15',0
  boot_enablint_ide db   'Habiliar interrupciones en controladores IDE',0
  boot_detectfloppy db   'Buscar unidades de disquete',0
  boot_detecthdcd   db   'Buscar discos duros y unidades ATAPI',0
  boot_getcache     db   'Tomar memoria para cach‚',0
  boot_detectpart   db   'Buscar particiones en discos',0
  boot_init_sys     db   'Inicializar directorio del sistema /sys',0
  boot_loadlibs     db   'Cargando librer¡as (.obj)',0
  boot_memdetect    db   'Determinando cantidad de memoria',0
  boot_tss          db   'Configurando TSSs',0
  boot_cpuid        db   'Leyendo CPUIDs',0
;  boot_devices      db   'Detectando dispositivos',0
  boot_setmouse     db   'Configurando el rat¢n',0
  boot_windefs      db   'Setting window defaults',0
  boot_bgr          db   'Calculating background',0
  boot_resirqports  db   'Reservando IRQs y puertos',0
  boot_setostask    db   'Configurando tarea OS',0
  boot_allirqs      db   'Desenmascarando IRQs',0
  boot_tsc          db   'Leyendo TSC',0
  boot_cpufreq      db   'La frequencia del CPU es ','    ',' MHz',0
  boot_pal_ega      db   'Configurando paleta EGA/CGA 320x200',0
  boot_pal_vga      db   'Configurando paleta VGA 640x480',0
  boot_failed       db   'Fallo al iniciar la primer aplicaci¢n',0
  boot_mtrr         db   'Configurando MTRR',0

  boot_APIC_found   db   'APIC habilitado', 0
  boot_APIC_nfound  db   'APIC no encontrado', 0
if preboot_blogesc
  boot_tasking      db   'Todo configurado - presiona ESC para iniciar',0
end if

msg_version    db 'versi¢n incompatible del controlador',13,10,0
msg_www        db 'por favor, visita www.kolibrios.org',13,10,0

ud_user_message db 'Error: instrucci¢n no soportada por el procesador',0
