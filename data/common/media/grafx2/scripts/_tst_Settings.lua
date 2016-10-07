-- Test LUA inputbox
--  this script tests the inputbox

w, h = getbrushsize()
--[[
messagebox(
  "Forecolor: " .. getforecolor() .. "\n" ..
  "Backcolor: " .. getbackcolor() .. "\n" ..
  "Transparent color: " .. gettranscolor() .. "\n" ..
  "Brush dimensions: " .. w .. "x" .. h
)
]]


ok, w, h = inputbox("Modify brush",
  "RGB",    1, 0, 1,  -1,
  "HSV",    0, 0, 1,  -1,
  "HSL",    0, 0, 1,  -1,
  "Width",  w, -900.0,900.0, 3,
  "Height", h, -900.0,900.0, 4,
  "X Flip", 0, 0, 1,  0,
  "Y Flip", 0, 0, 1,  0,
  "Degrees",1, 0, 1,  -2,
  "Radians",0, 0, 1,  -2  
);
if ok == true then
  messagebox(
    "w: " .. w .. "\n" ..
    "h: " .. h .. "\n"
  )
end


