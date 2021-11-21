package kos

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
func Event()(int)
func GetButtonID()(id int)
func CreateButton(x int, y int, xsize int, ysize int, id int, color uint32)
func Exit()
func Redraw(int)
func Window(y int, x int, w int,h int, title string)
func WriteText(x int ,y int , color uint32, text string)
func WriteText2(int ,int ,int, uint32,uint32)
func DrawLine(x1 int, y1 int, x2 int, y2 int, color uint32)(uint32)
func DrawBar(x int, y int, xsize int, ysize int, color uint32)
func DebugOutHex(uint32)
func DebugOutChar(byte)
func DebugOutStr(string)

func Pointer2byteSlice(ptr uint32) *[]byte __asm__("__unsafe_get_addr")

//func Pointer2uint32(ptr interface{}) uint32 __asm__("__unsafe_get_addr")