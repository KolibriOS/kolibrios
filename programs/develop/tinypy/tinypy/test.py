import kolibri
if __name__=="__main__":
  print("ok!")
  f = kolibri.open("/hd0/1/tinypy/test.txt", "a")
  l=[]
  s=""
  for i in range(10):
      s = s + 'A'
      l.append(s+'\n')
  print(l)
  f.writelines(l)
  f.close()
