package kernel
import "os"


func Load() {
x := "test"
	for true {
		switch os.Event() {
			
			case 1: 
				os.Redraw(1)				
				os.Window(50,250,450,200,x)
				os.Redraw(2)
				os.Sleep(100)
			case 3:
				os.Exit()
		}
	}
}
