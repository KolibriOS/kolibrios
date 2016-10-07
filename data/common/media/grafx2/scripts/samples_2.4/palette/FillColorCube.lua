--PALETTE: Fill ColorCube voids v1.0
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

--
-- Create a palette by continously filling the greatest void in the RGB color-cube
--


SHADES = 16 -- Going 24bit will probably be too slow and steal too much memory, so we're 12bit (4096 colors) for now

ini = 0
exp = 255

OK,ini,exp = inputbox("Fill Palette Color voids",
                       "From/Keep #: 0-254",    0,  0,254,0,
                       "Replace to #: 1-255",  31,  1,255,0
);


function initColorCube(sha)
  ary = {}
  for z = 0, sha-1, 1 do
   ary[z+1] = {}
   for y = 0, sha-1, 1 do
    ary[z+1][y+1] = {}
   end
  end
  return ary
end


function addColor2Cube(cube,sha,r,g,b) -- Gravity model
  star = 1000000
  fade = 1000

  cube[r+1][g+1][b+1] = star
  for z = 0, sha-1, 1 do
   for y = 0, sha-1, 1 do
    for x = 0, sha-1, 1 do

      d = fade / ( (x-b)^2 + (y-g)^2 + (z-r)^2 )

      if cube[z+1][y+1][x+1] ~= nil then
         cube[z+1][y+1][x+1] = cube[z+1][y+1][x+1] + d
         else
         cube[z+1][y+1][x+1] = d
      end

  end;end;end
end


function findVoid(cube,sha)
  weakest = 999999999999
   weak_i = {-1,-1,-1}
  for z = 0, sha-1, 1 do
   for y = 0, sha-1, 1 do
    for x = 0, sha-1, 1 do
 
      w = cube[z+1][y+1][x+1]
      if w <= weakest then weakest = w; weak_i = {z,y,x}; end
     
  end;end;end
  return weak_i[1],weak_i[2],weak_i[3]
end

--

if OK == true then

  cube = initColorCube(SHADES)
  -- Fill cube with initial colors
  for n = 0, ini-1, 1 do
    r,g,b = getcolor(n)
    div = SHADES
    addColor2Cube(cube,SHADES,math.floor(r/div),math.floor(g/div),math.floor(b/div))
  end

  if ini == 0 then -- With no inital color, some inital data must be added to the colorcube. 
    addColor2Cube(cube,SHADES,0,0,0)
    setcolor(0, 0,0,0) 
    ini = ini + 1
  end

  for n = ini, exp, 1 do
    r,g,b = findVoid(cube,SHADES)
    mult = 255 / (SHADES - 1)
    setcolor(n, r*mult,g*mult,b*mult)  
    addColor2Cube(cube,SHADES,r,g,b)
  end

end



