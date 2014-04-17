;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2014. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


  boot_initirq:     cp850   'Inicializar IRQ',0
  boot_picinit:     cp850   'Inicializar PIC',0
  boot_v86machine:  cp850   'Inicializar sistema V86',0
  boot_inittimer:   cp850   'Inicializar reloj del sistema (IRQ0)',0
  boot_initramdisk  cp850   'Initialize ramdisk',0
  boot_initapic:    cp850   'Prueba inicializar APIC',0
  boot_enableirq:   cp850   'Habilitar interrupciones 2, 13',0
  boot_disabling_ide:cp850   'Habiliar interrupciones en controladores IDE',0
  boot_enabling_ide:cp850   'Habilitar interrupciones en controladores IDE',0
  boot_set_int_IDE: cp850   'Configuración del controlador de interrupciones para el IDE',0
  boot_detectfloppy:cp850   'Buscar unidades de disquete',0
  boot_detecthdcd:  cp850   'Buscar discos duros y unidades ATAPI',0
  boot_getcache:    cp850   'Tomar memoria para caché',0
  boot_detectpart:  cp850   'Buscar particiones en discos',0
  boot_init_sys:    cp850   'Inicializar directorio del sistema /sys',0
  boot_loadlibs:    cp850   'Cargando librerías (.obj)',0
  boot_memdetect:   cp850   'Determinando cantidad de memoria',0
  boot_tss:         cp850   'Configurando TSSs',0
  boot_cpuid:       cp850   'Leyendo CPUIDs',0
;  boot_devices:     cp850   'Detectando dispositivos',0
  boot_setmouse:    cp850   'Configurando el ratón',0
  boot_windefs:     cp850   'Setting window defaults',0
  boot_bgr:         cp850   'Calculating background',0
  boot_resirqports: cp850   'Reservando IRQs y puertos',0
  boot_setostask:   cp850   'Configurando tarea OS',0
  boot_allirqs:     cp850   'Desenmascarando IRQs',0
  boot_tsc:         cp850   'Leyendo TSC',0
  boot_cpufreq:     cp850   'La frequencia del CPU es ','    ',' MHz',0
  boot_pal_ega:     cp850   'Configurando paleta EGA/CGA 320x200',0
  boot_pal_vga:     cp850   'Configurando paleta VGA 640x480',0
  boot_failed:      cp850   'Fallo al iniciar la primer aplicación',0
  boot_mtrr:        cp850   'Configurando MTRR',0

  boot_APIC_found:  cp850   'APIC habilitado', 0
  boot_APIC_nfound: cp850   'APIC no encontrado', 0
if preboot_blogesc
  boot_tasking:     cp850   'Todo configurado - presiona ESC para iniciar',0
end if

msg_version:   cp850 'versión incompatible del controlador',13,10,0
msg_www:       cp850 'por favor, visita www.kolibrios.org',13,10,0

ud_user_message:cp850 'Error: instrucción no soportada por el procesador',0
mtrr_user_message cp850 '"There is a problem with MTRR configuration.\nPerformance can be low" -dW',0
