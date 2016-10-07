--PICTURE: Rainbow - Dark to Bright v1.1
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/
-- Email: dawnbringer@hem.utfors.se
-- MSN:   annassar@hotmail.com
--


--dofile("dawnbringer_lib.lua") 
run("../libs/dawnbringer_lib.lua")
--> db.shiftHUE(r,g,b, deg)

w, h = getpicturesize()

for y = 0, h - 1, 1 do
  for x = 0, w - 1, 1 do

   -- Fractionalize image dimensions
   ox = x / w;
   oy = y / h;

   r = 255 * math.sin(oy * 2) 
   g = (oy-0.5)*512 * oy
   b = (oy-0.5)*512 * oy

   r, g, b = db.shiftHUE(r,g,b,ox * 360); 
 
   c = matchcolor(r,g,b)
 
   putpicturepixel(x, y, c);

  end
  updatescreen(); if (waitbreak(0)==1) then return; end
end


