--PALETTE: Expand Colors v1.0
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


-- Continously fill the greatest void in the area of the color-cube enclosed by (or along ramps of) initial colors
-- This algorithm will create lines of allowed colors (all ranges) in 3d colorspace and the pick
-- new colors from the most void areas (on any line). Almost like a Median-cut in reverse.
--
-- Rather than filling the colorcube symmetrically it adds intermediate colors to the existing ones.
--
-- Running this script on the C64 16-color palette might be educational
--
--
--   Source cols#, Expand to #, 
--   Ex: 15-31 means that palette colors 0-15 is expanded to 16 new colors placed at slots 16-31
--
--    Spread mode: OFF - New colors will conform to the contrast & saturation of original colors 
--                       (new colors will stay on the ramps possible from the original colors)
--
--                 ON - New colors will expand their variance by each new addition (mostly notable when adding many new colors)
--                      Will add range lines/ramps to all new colors from old ones, but keep within max/min values of the
--                      original colors. 15-bit mode will dampen the spread towards extreme colors (if starting with low contrast)
--
--  15-bit colors: Higher color-resolution, 32768 possible colors rather than the 4096 of 12bit. Slower but perhaps better. 
--

SHADES = 16 -- Going 24bit will probably be too slow and steal too much memory, so start with 12bit (4096 colors) for now

ini = 0
exp = 255

OK,ini,exp,linemode,fbit = inputbox("Expand Colors (0-255):",
                           "Source Cols #: 1-254",    15,  1,254,0,
                           "Expand to #: 2-255",       31,  2,255,0,
                           "Spread mode",       0,  0,1,0,
                           "15-bit colors (slow)", 0,  0,1,0   
);

if (fbit == 1) then SHADES = 32; end



function initColorCube(sha)
  ary = {}
  for z = 0, sha-1, 1 do
   ary[z+1] = {}
   for y = 0, sha-1, 1 do
    ary[z+1][y+1] = {}
      for x = 0, sha-1, 1 do
        ary[z+1][y+1][x+1] = {false,0}
      end
   end
  end
  return ary
end

-- Gravity model (think of colors as stars of equal mass/brightness in a 3d space)
function addColor2Cube(cube,sha,r,g,b) 
  star = 1000000
  fade = 1000

  cube[r+1][g+1][b+1] = {false,star}

  for z = 0, sha-1, 1 do
   for y = 0, sha-1, 1 do
    for x = 0, sha-1, 1 do

      d = fade / ( (x-b)^2 + (y-g)^2 + (z-r)^2 )

      cube[z+1][y+1][x+1][2] = cube[z+1][y+1][x+1][2] + d

  end;end;end
end


-- Create new allowed colorlines in colorspace (ramps from which colors can be picked)
function enableRangeColorsInCube(cube,sha,r1,g1,b1,r2,g2,b2) 

    local div,r,g,b
    div = 256 / sha
    rs = (r2 - r1) / sha / div 
    gs = (g2 - g1) / sha / div
    bs = (b2 - b1) / sha / div

    for n = 0, sha-1, 1 do

     r = math.floor(r1/div + rs * n)
     g = math.floor(g1/div + gs * n)
     b = math.floor(b1/div + bs * n)

     cube[r+1][g+1][b+1][1] = true

    end
end


function findVoid(cube,sha)
  weakest = 999999999999
   weak_i = {-1,-1,-1}
  for z = 0, sha-1, 1 do
   for y = 0, sha-1, 1 do
    for x = 0, sha-1, 1 do
 
      c = cube[z+1][y+1][x+1]
      if c[1] == true then
        w = c[2]
        if w <= weakest then weakest = w; weak_i = {z,y,x}; end
      end 
    
  end;end;end
  return weak_i[1],weak_i[2],weak_i[3]
end

--

if OK == true then

  cube = initColorCube(SHADES)

  -- Define allowed colorspace
  for y = 0, ini-1, 1 do
    r1,g1,b1 = getcolor(y)
    for x = y+1, ini, 1 do
      r2,g2,b2 = getcolor(x)
      enableRangeColorsInCube(cube,SHADES,r1,g1,b1,r2,g2,b2)
    end
  end

  div = 256 / SHADES

  -- Fill cube with initial colors
  for n = 0, ini, 1 do
    r,g,b = getcolor(n)
    addColor2Cube(cube,SHADES,math.floor(r/div),math.floor(g/div),math.floor(b/div))
  end


  for n = ini+1, exp, 1 do
    r,g,b = findVoid(cube,SHADES)

    if (r == -1) then messagebox("Report:","No more colors can be found, exit at "..n); break; end

    mult = 255 / (SHADES - 1)
    setcolor(n, r*mult,g*mult,b*mult)  

    if linemode == 1 then
       -- Add lines from new color to all old  
       for x = 0, n-1, 1 do
          r2,g2,b2 = getcolor(x)
          enableRangeColorsInCube(cube,SHADES,r*mult,g*mult,b*mult,r2,g2,b2) -- uses 24bit values rgb
       end
    end
    
    addColor2Cube(cube,SHADES,r,g,b) -- rgb is in 'shade' format here 
   
  end

end



