
macro set_event_mask mask {
   mov	ebx, mask
   mov	eax, 40
   int	0x40
}

macro wait_for_event {
   mov	eax, 10
   int	0x40
}

macro check_for_event {
   mov	eax, 11
   int	0x40
}

macro wait_for_event_timeout timeout {
   mov	ebx,timeout
   mov	eax, 23
   int	0x40
}


event_redraw	 equ 1 shl 0
event_keyboard	 equ 1 shl 1
event_button	 equ 1 shl 2
event_background equ 1 shl 4
event_mouse	 equ 1 shl 5
event_ipc	 equ 1 shl 6
event_network	 equ 1 shl 7
event_debug	 equ 1 shl 8
event_irq0	 equ 1 shl 15
event_irq1	 equ 1 shl 16
event_irq2	 equ 1 shl 17
event_irq3	 equ 1 shl 18
event_irq4	 equ 1 shl 19
event_irq5	 equ 1 shl 20
event_irq6	 equ 1 shl 21
event_irq7	 equ 1 shl 22
event_irq8	 equ 1 shl 23
event_irq9	 equ 1 shl 24
event_irq10	 equ 1 shl 25
event_irq11	 equ 1 shl 26
event_irq12	 equ 1 shl 27
event_irq13	 equ 1 shl 28
event_irq14	 equ 1 shl 29
event_irq15	 equ 1 shl 30

