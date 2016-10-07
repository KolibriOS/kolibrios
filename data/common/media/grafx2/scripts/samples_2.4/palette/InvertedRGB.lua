--PALETTE Modify: Inverted RGB 
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



for c = 0, 255, 1 do

  r,g,b = getcolor(c)
  
  r2 = (g+b)/2
  g2 = (r+b)/2
  b2 = (r+g)/2

  setcolor(c, r2,g2,b2)

end
