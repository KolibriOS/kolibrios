--PALETTE Adjust: Shift Hue v0.9
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


--Shift_degrees = 45

OK,Shift_degrees = inputbox("Shift Hue v0.9","Degrees", 45, 0,360,3);


--
function shiftHUE(r,g,b,deg) -- V1.3 R.Fhager 2007, adopted from Evalion
 local c,h,mi,mx,d,s,p,i,f,q,t
 c = {g,b,r}
 mi = math.min(r,g,b)
 mx = math.max(r,g,b); v = mx;
 d = mx - mi;
 s = 0; if mx ~= 0 then s = d/mx; end
 p = 1; if g ~= mx then p = 2; if b ~= mx then p = 0; end; end
 
 if s~=0 then
  h=(deg/60+(6+p*2+(c[1+p]-c[1+(p+1)%3])/d))%6; 
  i=math.floor(h);
  f=h-i;
  p=v*(1-s);
  q=v*(1-s*f);
  t=v*(1-s*(1-f));
  c={v,q,p,p,t,v}
  r = c[1+i]
  g = c[1+(i+4)%6]
  b = c[1+(i+2)%6]
 end

 return r,g,b
end
--

if OK == true then

  for c = 0, 255, 1 do
    r,g,b = getcolor(c)
    setcolor(c, shiftHUE(r,g,b,Shift_degrees))
  end

end