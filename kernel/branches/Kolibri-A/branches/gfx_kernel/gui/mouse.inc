uglobal
  mouseunder:
   times 32*32*3+1 db ?
  mousecomb = $ - mouseunder
   times 32*32*3+1 db ?
endg

 uglobal
  mousepointer:
   times 4608 db 0 ; 32*32*4+62 = 4286
 endg
