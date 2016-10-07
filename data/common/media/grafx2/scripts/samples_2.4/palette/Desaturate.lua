--PALETTE Adjust: Desaturate v1.1
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

-- This script was adopted from Evalion, a Javascript codecrafting/imageprocessing project
-- http://goto.glocalnet.net/richard_fhager/evalion/evalion.html 


-- Note: Negative values will work as INCREASED saturation, but I'm not sure if this function is 100% correct


--percent = 25

OK,percent = inputbox("Desaturate Palette","Percent %", 25, 0,100,0);

--
function desaturate(percent,r,g,b) -- V1.0 by Richard Fhager
 p = percent / 100
 a = (math.min(math.max(r,g,b),255) + math.max(math.min(r,g,b),0)) * 0.5 * p
 r = r + (a-r*p)
 g = g + (a-g*p)
 b = b + (a-b*p)
 return r,g,b
end
--

if OK == true then

  for c = 0, 255, 1 do
    setcolor(c, desaturate(percent,getcolor(c)))
  end

end