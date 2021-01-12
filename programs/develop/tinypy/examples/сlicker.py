# C-style window example
# Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv3

import ksys # KolibriOS syscalls
import bitwise # Bitwise operations for large numbers

my_button   = 2   # My button
exit_button = 1   # System exit button
number = 0        # Clicks count

colors = ksys.get_sys_colors() # Get system colors table

def Redraw():
    ksys.start_draw()
    ksys.create_window(10, 40, 400, 200, "My window", colors.work_area, 0x14)
    ksys.draw_text("KolibriOS TinyPy example", 15, 34, 0, bitwise.add(0x90000000, colors.work_text))
    ksys.create_button(150, 100 , 50, 100, my_button, colors.work_button)
    ksys.draw_text("Click!", 155, 115, 0, bitwise.add(0x91000000, colors.work_button_text))
    ksys.draw_text(str(number), 15,100, 0, bitwise.add(0x92000000, colors.work_text))
    ksys.end_draw()

if __name__=="__main__":
    ksys.debug_print("Start!\n") # Print "Start!" in debug board
    while True:
        event = ksys.get_event()
        if event == 1: # Redraw event
            Redraw()
        if event == 3: # Buttons event
            button = ksys.get_button() # Get clicked button number
            if button == exit_button:
                break;
            if button == my_button:
                number=number+1
                Redraw()
    print("Done!") # Print "Done!" in console
