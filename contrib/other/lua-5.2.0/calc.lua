-- simple calculator on LUA for KolibriOS

--init global variables
disp=0 --display
stack=0 --stack
will=0 --operation

function drawwin() 
--here we draw window
paintstart()
--begin redraw
window(10,10,153,180,65069280)
--define window
textout(3,13,0,"LuaCalc")
--print title
textout(6, 30, 0, disp)
--and display

--then we need make buttons and print labels of buttons
makebutton(6,60,20,20,17,13619151)
textout(9,63,0,"7")
makebutton(6,90,20,20,14,13619151)
textout(9,93,0,"4")
makebutton(6,120,20,20,11,13619151)
textout(9,123,0,"1")
makebutton(6,150,50,20,10,13619151)
textout(9,153,0,"0")


makebutton(36,60,20,20,18,13619151)
textout(39,63,0,"8")
makebutton(36,90,20,20,15,13619151)
textout(39,93,0,"5")
makebutton(36,120,20,20,12,13619151)
textout(39,123,0,"2")


makebutton(66,60,20,20,19,13619151)
textout(69,63,0,"9")
makebutton(66,90,20,20,16,13619151)
textout(69,93,0,"6")
makebutton(66,120,20,20,13,13619151)
textout(69,123,0,"3")


makebutton(96,60,20,20,20,13619151)
textout(99,63,0,"*")
makebutton(96,90,20,20,21,13619151)
textout(99,93,0,"/")
makebutton(96,120,20,20,22,13619151)
textout(99,123,0,"-")
makebutton(96,150,20,20,23,13619151)
textout(99,153,0,"+")

makebutton(126,60,20,20,30,13619151)
textout(129,63,0,"C")
makebutton(126,90,20,20,31,13619151)
textout(129,93,0,"CE")
makebutton(126,120,20,50,32,13619151)
textout(129,123,0,"=")

paintend()
--and finish redraw
end


--main loop
while 1==1 do --loop until exit
	event=waitevent() --check the event
	if event==1 then drawwin() end --redraw needed
	if event==2 then key=getkey()  end --get keyboard scancode
	if event==3 then button=getbutton() --button pressed
		if button==1 then sysexit() end --close button
		if button==10 then disp=disp*10 end --numerical buttons - 0
		if button==11 then disp=disp*10+1 end --1
		if button==12 then disp=disp*10+2 end --2
		if button==13 then disp=disp*10+3 end --3
		if button==14 then disp=disp*10+4 end
		if button==15 then disp=disp*10+5 end
		if button==16 then disp=disp*10+6 end
		if button==17 then disp=disp*10+7 end
		if button==18 then disp=disp*10+8 end 
		if button==19 then disp=disp*10+9 end --and 9
		if button==30 then disp=0  
		stack=0 end  -- C button - clear disp and memory
		if button==31 then disp=0 end -- CE button - clear disp
		if button==20 then will=1 
		stack=disp
		disp=0		
		end -- next is multiple
		if button==21 then will=2 
		stack=disp
		disp=0		
		end --next is divison
		if button==22 then will=3 
		stack=disp
		disp=0		
		end -- next is substraction
		if button==23 then will=4 
		stack=disp
		disp=0
		end -- next is addition
		if button==32 then --evalute
		if will==1 then disp=stack*disp end 
		if will==2 then disp=stack/disp end 
		if will==3 then disp=stack-disp end
		if will==4 then disp=stack+disp end
		end
		drawwin() --redraw... we need it after pressing buttons
	end
end
