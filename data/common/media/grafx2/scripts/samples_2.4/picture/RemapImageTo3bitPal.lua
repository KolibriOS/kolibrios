--SCENE: Remap pic to 3bit, LineEDdith. (Same line simple error-diffusion dither)
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

--
-- Just a demonstration.
--



power = 0.6

-- Channel shades (shades = 2 ^ bit-depth)
shades = 2

mult = 255 / (shades-1)


colors = {}
col = 0
for r = 0, shades-1, 1 do
  for g = 0, shades-1, 1 do
    for b = 0, shades-1, 1 do
       col = col + 1
       colors[col] = { r*mult, g*mult, b*mult } 
    end
  end
end


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

      rn = re + r
      gn = ge + g
      bn = be + b

      n = matchcolor(rn,gn,bn);

    putpicturepixel(x, y, n);

    
    rn,gn,bn = getcolor(getpicturepixel(x,y));
   
    re = (re + (r - rn)) * power
    ge = (ge + (g - gn)) * power
    be = (be + (b - bn)) * power

   
  end
end
