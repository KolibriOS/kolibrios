import kolibri

def onshow():
    print("Show window")

def onkey():
    print("Key pressed")

def onbtn():
    print("Button pressed")


kolibri.debug_print("Debug test line\n")
w = kolibri.window(10,10,400, 400, False)
w.on_show = onshow
w.on_key = onkey
w.on_button = onbtn
w.show()
print("running")
w.run()
print("Exit")
