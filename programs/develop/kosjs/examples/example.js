var button_text = 0
var button = 2

function Redraw()
{
    StartDraw()
    WindowCreate(10, 40, 400, 200, "My window", 0xFFFFFF, 0x14)
    WriteText("KolibriOS JS example", 15, 34, 0, 0x90000000, 0xFFFFFF)
    ButtonCreate((150 << 16) + 100, (100 << 16) + 50, button, 0x177245)
    WriteText("Click!", 155,115, 0, 0x91000000 | 0xFFFFFF)
    WriteText(button_text, 15,100, 0, 0x92000000)
    EndDraw()
}

while(1)
{
    var gui_event = GetEvent()
    switch(gui_event)
    {
        case 0:
            break
        case 1:
            Redraw()
            break
        case 3:
            var pressed_button = GetButtonEvent()
            switch (pressed_button)
            {
              case 1:
                Exit()
                break
              case button:
                button_text++;
                Redraw()
                break
            }
            break
    }
}

