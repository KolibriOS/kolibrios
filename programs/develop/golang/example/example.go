package example

import (
	"colors"
	"../kos"
)

const (
	Btn1    = 2
	Btn2    = 3
	BtnExit = 1
)

type Button struct { // structure gui button
	label string
	x     int
	y     int
	id    int
}

func NewButton() Button {
	object := Button{"Text", 0, 0, Btn1} // default data
	return object
}

func (button *Button) make() {
	kos.CreateButton(button.x, button.y, len(button.label)*15, 30, button.id, colors.Blue)
	kos.WriteText(button.x, button.y, 0x11000000|colors.White, button.label)
}

func RedrawAll(bar_pos int) {
	kos.Redraw(1)
	kos.Window(500, 250, 420, 200, "Example GoLang")
	kos.DrawLine(32, 80, 150, 80, colors.Green)
	kos.DrawBar(bar_pos, 90, 100, 30, colors.Red)

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
	var pos = 160
	time := kos.GetTime()
	kos.DebugOutStr("Time: ")
	kos.DebugOutHex(time)
	for {
		switch kos.Event() {
		case kos.EVENT_REDRAW:
			RedrawAll(pos)
			break
		case kos.EVENT_BUTTON:
			switch kos.GetButtonID() {
			case Btn1:
				pos -= 32
				RedrawAll(pos)
				break
			case Btn2:
				pos += 32
				RedrawAll(pos)
				break
			case BtnExit:
				kos.Exit()
			}
		}
	}
}
