import kolibri

def onshow():
    print("Show window")

def onkey():
    print("Key pressed")

def onbtn():
    print("Button pressed")

if __name__=="__main__":
  w = kolibri.window(10,10,400, 400, False)
  w.on_show = onshow
  w.on_key = onkey
  w.on_button = onbtn
  w.show()
  print("running")
  w.run()
  print("Exit")
