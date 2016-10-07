--SCENE: Remap pic 2 RGB, 1lineED-dith. (Same line simple error-diffusion dither)
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>


power = 0.615

c1 = 0.8 -- Error weight (white is green)
c2 = 0.2 -- RGB weight (white is r+g+b)

-- Set Palette (to a predefined one)

colors = {{  0,  0,  0},
          {255,  0,  0},
          {  0,255,  0},
          {  0,  0,255}
         }


chm = {1,0,0} 

for c = 1, #colors, 1 do
  setcolor(c-1,colors[c][1],colors[c][2],colors[c][3]) 
end

for c = #colors, 255, 1 do
  setcolor(c,0,0,0) 
end



w, h = getpicturesize()

for y = 0, h - 1, 1 do

  re = 0
  ge = 0
  be = 0

  for x = (y%2), w - 1, 1 do
    
    r,g,b = getbackupcolor(getbackuppixel(x,y));

      rn = re * c1 + r * chm[1+(y+0+x)%3] * c2
      gn = ge * c1 + g * chm[1+(y+1+x)%3] * c2
      bn = be * c1 + b * chm[1+(y+2+x)%3] * c2

      n = matchcolor(rn,gn,bn);

    putpicturepixel(x, y, n);

    
    rn,gn,bn =  getcolor(getpicturepixel(x,y));
   
    re = (re + (r - rn)) * power
    ge = (ge + (g - gn)) * power
    be = (be + (b - bn)) * power

    

  end
end
