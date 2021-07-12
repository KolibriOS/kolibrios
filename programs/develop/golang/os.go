package os

const (
    EVENT_NONE = 0     /* Event queue is empty */
    EVENT_REDRAW = 1   /* Window and window elements should be redrawn */
    EVENT_KEY = 2      /* A key on the keyboard was pressed */
    EVENT_BUTTON = 3   /* A button was clicked with the mouse */
    EVENT_DESKTOP = 5  /* Desktop redraw finished */
    EVENT_MOUSE = 6    /* Mouse activity (movement, button press) was detected */
    EVENT_IPC = 7      /* Interprocess communication notify */
    EVENT_NETWORK = 8  /* Network event */
    EVENT_DEBUG = 9    /* Debug subsystem event */
    EVENT_IRQBEGIN = 16
)
func Sleep(uint32)
func GetTime()(time uint32)
func Event()(uint32)
func GetButtonID()(id int)
func CreateButton(x uint32, y uint32, xsize uint32, ysize uint32, id uint32, color uint32)
func Exit()
func Redraw(uint32)
func Window(y uint32, x uint32, w uint32,h uint32, title string)
func WriteText(x uint32 ,y uint32 , color uint32, text string)
func WriteText2(uint32 ,string ,uint32, uint32,uint32)
func DrawLine(x1 uint32, y1 uint32, x2 uint32, y2 uint32, color uint32)(uint32)
func DrawBar(x int, y int, xsize int, ysize int, color uint32)
func DebugOutHex(uint32)
func DebugOutChar(byte)
func DebugOutStr(string)
