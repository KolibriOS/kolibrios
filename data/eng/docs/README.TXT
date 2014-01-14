              ************************
              * Kolibri OS  0.7.7.0+ *
              *    February 2010     *
              ************************
 
   The latest release of the OS is available on the official site:
            http://kolibrios.org
 
   If you have questions - look for support on our forum:
            http://board.kolibrios.org (rus+eng)
 
             ***********************************
             *  What is new in this release?   *
             ***********************************
 
    The following changes, revisions, fixes in the kernel have been made:
 
  From Evgeny Grechnikov (Diamond) - Russian Federation
    1) Improvement of drives detection using BIOS service (V86)
    2) Kernel bugfixes.
 
  From Sergey Semyonov (Serge) - Russian Federation
    1) Driver for ATI videocards updated.
    2) Linux-like kernel mutexes
 
  From Mihail Semenyako (mike.dld) - Republic of Belarus
    1) Refactoring of window subsystem.
 
  From <Lrz>
    1) Processing API functions in order to completely remove the shift-register-call
    2) Refactoring, optimization, bugfixes of some places in the kernel.
 
  From A.Jerdev (art_zh) - United Kingdom
    1) Memory-mapped I/O (MMIO) access from the userworld.
    2) PCIe Extended configuration space access (auto-adjust for AMD-NPT since Athlon64).
 
  From turbanoff
    1) Read from ext2fs/ext3fs.
 
  From Marat Zakiyanov (Mario79) - Russian Federation
    1) Improvements in the processing of mouse clicks.
 
  From hidnplayr
    1) Rewriting of the entire network subsystem.
 
  From Asper
   1) Drivers for the sound cards VT823(X) and EMU10K1X.
 
  From tsdima
    1) Driver for the sound card ForteMedia fm801.
 
  From Maxis
    1) changes in free memory checks.
 
 
 

 
    The following changes, revisions, fixes in applications have been made:
 
   * New versions of applications and new applications:
 
    VIEW3DS  - macgub, version 0.054
    FASM     - Pavel Rymovski (Heavyiron), updated to version 1.69.11
               IgorA, use the library box_lib, russification,
               Marat Zakiyanov (Mario79) fix bugs encountered
               Evgeny Grechnikov (Diamond), fix of window wrapping
    PCIDEV   - art_zh,  MMIO sample dumps added;  vendors database
                trimmed to reduce the distro size. 
    BOARD    - Evgeny Grechnikov (Diamond), do not throw out existing
                messages at the program start
    UNVWATER - Asper, demo ported from OctaOS
                (the author is Octavio Vega Fernandez).
    HEED     - staper, version 0.14: use the library box_lib;
                Ctrl+C/X/V, Ctrl+B to select a block
               Marat Zakiyanov (Mario79), using of OpenDialog component,
                little optimization of the code    
    SHELL    - Albom, version 0.4.2
    CPU      - Evgeny Grechnikov (Diamond) at the suggestion of
               Kirill Lipatov (Leency), do not show system processes
                by default
    HTMLV    - Kirill Lipatov (Leency), version 0.62a
    EOLITE   - Kirill Lipatov (Leency), version 0.98.7a
    RTFREAD  - Sorcerer, cosmetic fix
               dunkaist, menu_bar and scroll_bar from box_lib, 'end' hotkey
    E80      - Albom, version 0.5.1
    KIV      - Evgeny Grechnikov (Diamond), file name in the window
                title, configurable keyboard shortcuts for next/prev
                images in the current folder
    SUDOKU   - staper, port of Sudoku game - that is a puzzle with digits, where you
                have to fill free cells with digits from 1 to 9 so that every row,
                every column and every small square 3x3 contain each digit just one time.
                There are 9 levels in the game.
    REVERSI  - staper, port of strategy game Reversi (Othello)
    MSQUARE  - staper, math puzzle "magic square": sum of the numbers in rows, columns 
                (less often in diagonals) should be equal
    FINDN-S  - Artemonische, a simple game in which you have to collect all the numbers 
                in the order of 10 to 90
    PANEL    - Evgeny Grechnikov (Diamond), a little change in the panel clock
                and reanimation of backlight on the Alt+[Shift+]Tab press.
    AC97SND  - Asper, support of playlists in m3u format.
    @NOTIFY  - SoUrcerer, program for popup text messages
    DOSBOX   - Evgeny Grechnikov (Diamond), version 0.74
    ASCIIVJU - Konstantin Dutkevich (Nazarus), program shows ASCII table 
    PIPETKA  - Rock_maniak_forever, program to get color of a pixel on the screen
    SNAKE    - dunkaist, yet another implementation of classic game
	DOWNLOADER	- Updated to work with new stack by hidnplayr, bugfixes by mario
	FTPC	 - Command-line FTP client from hidnplayr
	FTPD	 - New FTP daemon with ini-based config from hidnplayr
	PING	 - ICMP echo (ping) program from hidnplayr
	IRCC     - New IRC client from hidnplayr
	NETCFG   - Network configuration utility from hidnplayr
	NETSTAT  - Network status program from hidnplayr
	NSLOOKUP - Name service lookup utility from CleverMouse
	SYNERGYC - Synergy software keyboard/mouse sharing client from hidnplayr
	TELNET	 - New telnet client from hidnplayr
	TFTPC	 - New TFTP client from hidnplayr
	ZEROCONF - Updated to work on new stack, bugfixes by 
	LOADDRV  - New program, loads the driver of which the name was given as a parameter,
				from hidnplayr
	
 
 
   * New versions of dynamic libraries and new dynamic libraries:
    libini   - Evgeny Grechnikov (Diamond), added function to read
                keyboard shortcuts
    box_lib  - IgorA, update component tree_list, new component
                text_editor, documentation on the library
               Marat Zakiyanov (Mario79) refinement of OpenDialog component, rewriting 
                of some applications to use it, new component PathShow
	console	 - numpad support and some escape code support from hidnplayr
			   bugfixes by Serge, SoUrcerer and Leency
	http	 - HTTP library from hidnplayr
	
	
	
	* New versions of drivers and new drivers:
	sound.obj	- service to detect the soundcard and load correct driver
				   by hidnplayr
	3c59x.obj	- 3Com network driver, converted to new stack by hidnplayr
	dec21x4x.obj- New DEC network driver (for VirtualPC), written by hidnplayr
	i8254x.obj  - New intel eepro/1000 network driver written by hidnplayr
	i8255x.obj  - intel eepro/100 network driver, converted to new stack by hidnplayr
	mdt80x.obj  - Myson mtd80x network driver, written by hidnplayr
	pcnet32.obj - AMD pcnet driver, converted to new stack by hidnplayr
	r6040.obj   - New RDC R6040 network driver, written by hidnplayr and asper
	rhine.obj   - New VIA Rhine network driver, written by hidnplayr and asper
	rtl8029.obj - Realtek 8029 network driver, converted to new stack by hidnplayr
	rtl8139.obj - Realtek 8139 network driver, converted to new stack by hidnplayr
	rtl8169.obj - Realtek 8169 network driver, converted to new stack by hidnplayr
	sis900.obj  - SIS 900 network driver, converted to new stack by hidnplayr
 


/-----------------------------------------------\
* Dates of publication of the distribution kits *
\-----------------------------------------------/

RE N1           30.08.2003
RE N2           07.10.2003
RE N3           26.11.2003
RE N4           23.12.2003
RE N5           15.02.2004
RE N6           27.03.2004
KOLIBRI N1      16.05.2004
RE N7           11.06.2004
KOLIBRI N2      28.08.2004
RE N8           01.12.2004
KOLIBRI N3      22.02.2005  
        Beta 2: 20.03.2005
KOLIBRI N4      07.06.2005
KOLIBRI 0.5.0.0 04.10.2005
        0.5.1.0 12.10.2005
        0.5.2.0 02.12.2005
        0.5.3.0 18.03.2006
        0.5.8.0 09.07.2006
        0.5.8.1 25.07.2006

        0.6.0.0 04.09.2006
        0.6.3.0 31.10.2006
        0.6.5.0 14.02.2007

        0.7.0.0 07.06.2007
        0.7.1.0 23.09.2007
        0.7.5.0 31.01.2009
        0.7.7.0 13.12.2009

/----------------\
* KolibriOS TEAM *
\----------------/

This list contains all, who has actively helped to creation and development
of KolibriOS, whoever possible.
 (people are enumerated in the order by time of participation in the project,
  from bottom to top - from past to future, through present)

* Trans                                \
* VaStaNi                              |
* Ivan Poddubny                        |
* Marat Zakiyanov (Mario79)            |
* Mihail Semenyako (mike.dld)          |  system programming
* Sergey Kuzmin (Wildwest)             |
* Andrey Halyavin (halyavin)           |  loaders,
* Mihail Lisovin (Mihasik)             |  kernel improvements and so on
* Andrey Ignatiev (andrew_programmer)  |
* NoName                               |
* Evgeny Grechnikov (Diamond)          |
* Iliya Mihailov (Ghost)               |
* Sergey Semyonov (Serge)              |
* Johnny_B                             |
* kasTIgar                             |
* SPraid                               |
* Rus                                  |
* Alver                                |
* Maxis                                |
* Galkov                               |
* CleverMouse                          |
* tsdima                               |
* turbanoff                            |
* Asper                                |
* art_zh                               /

* Mihail Lisovin (Mihasik)             \
* Andrey Ivushkin (Willow)             |
* Mihail Semenyako (mike.dld)          |
* Pavlushin Evgeny (Exis)              |
* Ivan Poddubny                        |  application programming
* Marat Zakiyanov (Mario79)            |
* Sergey Kuzmin (Wildwest)             |
* Andrey Halyavin (halyavin)           |  creation of new,
* Hex                                  |  port of existing
* Andrey Ignatiev (andrew_programmer)  |  or revisions of old
* ealex                                |  applications for Kolibri
* Olaf                                 |
* Evgeny Grechnikov (Diamond)          |
* Navanax                              |
* Johnny_B                             |
* Pavel Rymovski (Heavyiron)           |
* Vitaly Bendik (mistifi(ator)         |
* Iliya Mihailov (Ghost)               |
* Maxim Evtihov (Maxxxx32)             |
* Vladimir Zaitsev (Rabid Rabbit)      |
* vectoroc                             |
* Alexey Teplov (<Lrz>)                |
* Sergey Semyonov (Serge)              |
* YELLOW                               |
* iadn                                 |
* Maciej Guba (macgub)                 |
* Mario Birkner (cYfleXX)              |
* hidden player (hidnplayr)            |
* trolly                               |
* nilgui                               |
* kaitz                                |
* DedOk                                |
* SPraid                               |
* Rus                                  |
* Alver                                |
* Dron2004                             |
* Gluk                                 |
* Aleksandr Bogomaz (Albom)            |
* Kirill Lipatov (Leency)              |
* Vasiliy Kosenko (vkos)               |
* IgorA                                |
* staper                               |
* chaykin                              |
* Alexander Meshcheryakov              |
    (Self-Perfection)                  |
* CleverMouse                          |
* tsdima                               |
* art_zh                               |
* Asper                                |
* Pterox                               |
* Artemonische                         |
* dunkaist                             /

* Hex                                  \
* Diamond                              /  documentation

* CodeWorld                            \  forum http://meos.sysbin.com
* mike.dld                             /  site http://kolibrios.org; svn-server

* Alexey Teplov (<Lrz>)                \          (KolibriOS logo)
* goglus                               |  design  (KolibriOS background)
* Kirill Lipatov (Leency)              /          (KolibriOS icons)

* Pavel Rymovski (Heavyiron)           \
* Vitaly Bendik (mistifi(ator)         |
* vectoroc                             |
* Veliant                              |  testing,
* AqwAS                                |  countenance
* Mike                                 |
* camper                               |
* Dmitry the Sorcerer                  |
* Ataualpa                             |
* Maxis                                |
* Galkov                               |
* ChE                                  /

and others...

						KolibriOS team
