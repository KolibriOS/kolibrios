match =intel x, cpu {
 include 'language.inc'
}

include 'memory.inc'
include 'text.inc'

match x =windows, cpu
 { include 'draw.inc' }

match x =kolibri, cpu
 { include 'drawk.inc' }

match x =windows, cpu
 { include 'image.inc' }

match x =kolibri, cpu
 { include 'imagek.inc' }

include 'font.inc'

match x =windows, cpu
 { include 'system.inc' }

match x =kolibri, cpu
 { include 'systemk.inc' }