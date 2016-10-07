--PICTURE: Pattern - Sierpinsky triangle v1.0
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/
-- Email: dawnbringer@hem.utfors.se
-- MSN:   annassar@hotmail.com
--
-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

-- This script was adopted from Evalion, a Javascript codecrafting/imageprocessing project
-- http://goto.glocalnet.net/richard_fhager/evalion/evalion.html 
--

frac = {{1,1},{1,0}}

iter = 15

--
function pattern(x,y,p,n,i) -- Fractal Pattern V1.0 by Richard Fhager (mod allows for wrapping)
 py = #p
 px = #p[1]
 while ((p[1+math.abs(math.floor(y*py))%py][1+math.abs(math.floor(x*px))%px]) == 1 and n<i) do
  x=x*px-math.floor(x*px); 
  y=y*py-math.floor(y*py);
  n = n+1
 end
 return 1 - n/i;
end
--

w, h = getpicturesize()

rp,gp,bp = getcolor(getforecolor())

for y = 0, h - 1, 1 do
  for x = 0, w - 1, 1 do

    ox = x / w;
    oy = y / h;

    f = pattern(ox,oy,frac,0,iter);

    c = matchcolor(rp*f,gp*f,bp*f)

    putpicturepixel(x, y, c);

  end
  updatescreen()
  if (waitbreak(0)==1) then
    return
  end
end

