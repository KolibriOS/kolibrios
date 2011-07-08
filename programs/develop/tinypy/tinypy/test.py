import kolibri
if __name__=="__main__":
  print("ok!")
  f = kolibri.open("/hd0/1/tinypy/test.txt")
  print(f.size)
  print(f.read())
  f.close()
