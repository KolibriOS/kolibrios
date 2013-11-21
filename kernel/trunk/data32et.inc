  boot_initirq       latin1   'Algväärtustan IRQ',0
  boot_picinit       latin1   'Algväärtustan PIC',0
  boot_v86machine    latin1   'Algväärtustan süsteemi V86 masinat',0
  boot_inittimer     latin1   'Algväärtustan süsteemi taimerit (IRQ0)',0
  boot_initramdisk   latin1   'Initialize ramdisk',0
  boot_initapic      latin1   'Proovin Algväärtustada APIC',0
  boot_enableirq     latin1   'Luban katkestused 2, 13',0
  boot_disabling_ide latin1   'Keelan IDE kontrolleri katkestused',0
  boot_enabling_ide  latin1   'Luban IDE kontrolleri katkestused',0
  boot_set_int_IDE   latin1   'Määran IDE kontrolleri halduri',0
  boot_detectfloppy  latin1   'Otsin floppi kettaid',0
  boot_detecthdcd    latin1   'Otsin kõvakettaid ja ATAPI seadmeid',0
  boot_getcache      latin1   'Küsin puhvri mälu',0
  boot_detectpart    latin1   'Otsin kettaseadmete partitsioone',0
  boot_init_sys      latin1   'Algväärtustan süsteemi kataloogi /sys',0
  boot_loadlibs      latin1   'Laadin mooduleid (.obj)',0
  boot_memdetect     latin1   'Avastan mälu mahtu',0
  boot_tss           latin1   'Määran TSSe',0
  boot_cpuid         latin1   'Loen CPUIDd',0
;  boot_devices      db   'Detecting devices',0
  boot_setmouse      latin1   'Seadistan hiirt',0
  boot_windefs       latin1   'Seadistan akende vaikeväärtusi',0
  boot_bgr           latin1   'Kalkuleerin tausta',0
  boot_resirqports   latin1   'Reserveerin IRQsi ja porte',0
  boot_setostask     latin1   'Seadistan OS protsessi',0
  boot_allirqs       latin1   'Unmasking IRQs',0
  boot_tsc           latin1   'Loen TSC',0
  boot_cpufreq       latin1   'CPU sagedus on ','     ',' MHz',0
  boot_pal_ega       latin1   'Seadistan EGA/CGA 320x200 paletti',0
  boot_pal_vga       latin1   'Seadistan VGA 640x480 paletti',0
  boot_failed        latin1   'Esimese programmi käivitamine ebaõnnestus',0
  boot_mtrr          latin1   'Määran MTRR',0

  boot_APIC_found    latin1   'APIC aktiveeritud', 0
  boot_APIC_nfound   latin1   'APIC ei leitud', 0
if preboot_blogesc
  boot_tasking       latin1   'Kõik valmis - vajuta ESC alustamiseks',0
end if