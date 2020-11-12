package kernel
import "os"
import "colors"

const (
    Btn1=2;
    Btn2=3;
    BtnExit=1;
)

func RedrawAll(bar_pos int){
    btn_txt1:=" <- "
    btn_txt2:=" -> "
    str:="I love KolibriOS"
    title:="Test Golang"            
    os.Redraw(1)	
    os.Window(500,250,420,200, title)
    os.WriteText(32,32, 0x11000000 | colors.Green,str, 10)
    os.DrawLine(32, 80, 150, 80, colors.Green)
    os.CreateButton(32, 128, 80, 30, Btn1, colors.Blue);
    os.CreateButton(300, 128, 80, 30, Btn2, colors.Blue);
    os.WriteText(32,128, 0x11000000 | colors.White,btn_txt1, 10)
    os.WriteText(320,128, 0x11000000 | colors.White,btn_txt2, 10)
}

func Load() {
    time := os.GetTime()
    os.DebugOutStr("Time: ")
    os.DebugOutHex(time)
    var pos=32;
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
