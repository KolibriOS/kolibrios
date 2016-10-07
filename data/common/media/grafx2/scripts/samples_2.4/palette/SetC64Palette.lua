--PALETTE Set: C64 Palette (16 colors)
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>


OK,clean = inputbox("C64 Palette:", "Remove old palette", 0,  0,1,0 
);



 colors =  {{0,    0,  0}, -- 0  Black
            {62,  49,162}, -- 1  D.Blue
            {87,  66,  0}, -- 2  Brown
            {140, 62, 52}, -- 3  D.Red
            {84,  84, 84}, -- 4  D.Grey
            {141, 72,179}, -- 5  Purple
            {144, 95, 37}, -- 6  Orange
            {124,112,218}, -- 7  B.Blue
            {128,128,128}, -- 8  Grey
            {104,169, 65}, -- 9  Green
            {187,119,109}, -- 10 B.Red
            {122,191,199}, -- 11 Cyan
            {171,171,171}, -- 12 B.Grey 
            {208,220,113}, -- 13 Yellow
            {172,234,136}, -- 14 B.Green
            {255,255,255}  -- 15 White
           } 


if OK == true then

 for c = 1, #colors, 1 do
   setcolor(c-1,colors[c][1],colors[c][2],colors[c][3]) 
 end


 if clean == 1 then
   for c = #colors+1, 256, 1 do 
     setcolor(c-1,0,0,0)  
   end
 end

end