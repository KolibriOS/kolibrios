package kernel

import "os"
import "colors"

const (
    Btn1=2;
    Btn2=3;
    BtnExit=1;
)

type Button struct { // structure gui button
    label string
    x int
    y int
    id int
}

func NewButton() Button {
    object := Button{"Text",0,0,Btn1} // default data
    return object
}

func (button *Button) make() {
    os.CreateButton(button.x, button.y, len(button.label)*15, 30, button.id, colors.Blue);
    os.WriteText(button.x,button.y, 0x11000000 | colors.White, button.label)
}

func RedrawAll(bar_pos int){
    os.Redraw(1)
    os.Window(500,250,420,200, "Example GoLang")
    os.DrawLine(32, 80, 150, 80, colors.Green)
    os.DrawBar(bar_pos, 90, 100, 30, colors.Red);

    b1 := NewButton()
    b1.label = " <- "
    b1.x = 32
    b1.y = 128
    b1.id = Btn1
    b1.make()

    b2 := NewButton()
    b2.label = " -> "
    b2.x = 310
    b2.y = 128
    b2.id = Btn2
    b2.make()
}

func Main() {
    var pos = 160;
    //time := os.GetTime()
    //os.DebugOutStr("Time: ")
    //os.DebugOutHex(time)
    for true {
        switch os.Event() {
			case os.EVENT_REDRAW:
                RedrawAll(pos)
                break
            case os.EVENT_BUTTON:
            switch os.GetButtonID() {
                case Btn1:
                    pos-=32
                    RedrawAll(pos)
                    break
                case Btn2:
                    pos+=32
                    RedrawAll(pos);
                    break
                case BtnExit:
                    os.Exit()
            }
		}
	}
}
