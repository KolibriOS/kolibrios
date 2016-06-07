;	 file 'pcirom.bin':0,0x49
	file 'pcirom.bin'


	db 13,10
	line = '0'
	block = 0
	repeat 7
	  db 'INIT_0',line,' => X"'
	  repeat 32
	    pos = 32-%
	    load a byte from pos+block
	    b = a mod 16
	    if b<10
	       c = b + '0'
	    else
	       c = b-10+'A'
	    end if
	    b = a / 16
	    if b<10
	      d = b + '0'
	    else
	      d = b-10+'A'
	    end if
	    db d, c
	  end repeat
	  db '",',13,10
	  block = block + 32
	  line = line + 1
	end repeat
