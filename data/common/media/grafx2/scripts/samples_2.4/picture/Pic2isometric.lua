--PICTURE (part of): 2 Isometric v0.1b
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


-- Color 0 is assumed to be the background
--

iso = {{0, 0, 1, 1, 1, 1, 0, 0},
       {1, 1, 1, 1, 1, 1, 1, 1},
       {2, 2, 1, 1, 1, 1, 3, 3},
       {2, 2, 2, 2, 3, 3, 3, 3},
       {2, 2, 2, 2, 3, 3, 3, 3},
       {2, 2, 2, 2, 3, 3, 3, 3},
       {0, 0, 2, 2, 3, 3, 0, 0}}

isowidth  = 8
isoheight = 7

xoff = 0.5
yoff = 0

xstep = 4
ystep = 2
zstep = 4

-- Part of screen from top-left (4 = 1/4)
xsize = 5
ysize = 4


w, h = getpicturesize()

xo = math.floor(w * xoff)

-- just don't render more than can be fittted right now
w = math.floor(w / xsize)
h = math.floor(h / ysize)



for y = 0, h - 1, 1 do
  for x = 0, w - 1, 1 do

    isox = x * xstep - y * xstep
    isoy = y * ystep + x * ystep
    
    cb = getbackuppixel(x,y)

    --
    if cb ~= 0 then

      r,g,b = getbackupcolor(cb);
      c1 = matchcolor(r,g,b);
      c2 = matchcolor(r+64, g+64, b+64);
      c3 = matchcolor(r-64, g-64, b-64);
      
      cols = {0,c1,c2,c3}

      for iy = 1, isoheight, 1 do
       for ix = 1, isowidth, 1 do
 
         i = iso[iy][ix]
         c = cols[i+1]   
         if i ~= 0 then putpicturepixel(xo + isox+ix-1, isoy+iy-1, c); end
         
       end
      end

    end
    --

  end
end




