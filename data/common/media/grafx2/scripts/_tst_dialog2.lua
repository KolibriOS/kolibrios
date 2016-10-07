--
-- test of GUI library
--
run("libs/gui.lua")

local counter = gui.label{x=10, y=54, value=0, format="% .3d"}
local form = gui.dialog{
    title="Dialogtest",
    w=100,
    h=150,
    counter,
    gui.button{ label="+",
      x=6, y=38, w=14, h=14, repeatable=true, click=function() 
      counter.value=counter.value+1;
      counter:render();
    end},
    gui.button{ label="-",
      x=26, y=38, w=14, h=14, repeatable=true, click=function()
      counter.value=counter.value-1;
      counter:render();
    end},
    gui.button{ label="Help",
      x=6, y=70, w=54, h=14, click=function()
      messagebox("Help screen");
    end},
    gui.button{ label="Close",
      x=6, y=18, w=54, h=14, key=27, click=function()
      return true; -- causes closing
    end},
}

form:run()

