--BRUSH: Find AA-colors from pencolors
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

cellw = 8
cellh = 4
colors = 256

setbrushsize(cellw * 3, cellh * 3)


--
function makePalList(cols)
 pal = {}
 for n = 0, cols-1, 1 do
   r,g,b = getcolor(n)
   pal[n+1] = {r,g,b}
 end
 return pal
end
--

--
function getBestPalMatchHYBRID(rgb,pal,briweight)
 local diff,diffC,diffB,best,bestcol,cols,n,c,r,g,b,p,obri,pbri
 cols = #pal
 bestcol = 0
 best = 9e99

 r = rgb[1]
 g = rgb[2] 
 b = rgb[3]

 obri = math.pow(r*9,2)+math.pow(g*16,2)+math.pow(b*8,2)

 for n=0, cols-1, 1 do
  p = pal[n+1]
  pbri = math.pow(p[1]*9,2)+math.pow(p[2]*16,2)+math.pow(p[3]*8,2)
  diffB = math.abs(obri - pbri)


  diffC = (math.pow(r-p[1],2)+math.pow(g-p[2],2)+math.pow(b-p[3],2)) * 400
  --diff  = diffB + diffC
  diff = briweight * (diffB - diffC) + diffC
  if diff <= best then bestcol = n; best = diff; end
 end 

 return bestcol
end
--

--
function drawRectangle(x1,y1,w,h,c)
   for y = y1, y1+h, 1 do
    for x = x1, x1+w, 1 do
       putbrushpixel(x,y,c);
    end
   end
end
--



palList = makePalList(colors)

cf = getforecolor()
cb = getbackcolor()
rf,gf,bf = getcolor(cf)
rb,gb,bb = getcolor(cb)

ra = (rf + rb) / 2
ga = (gf + gb) / 2
ba = (bf + bb) / 2

rgb1 = {ra,ga,ba}
c1 = getBestPalMatchHYBRID(rgb1,palList,0.0)
c2 = getBestPalMatchHYBRID(rgb1,palList,0.75)
c3 = getBestPalMatchHYBRID(rgb1,palList,0.99)

q = {{cf,c1,cb},
     {cf,c2,cb},
     {cf,c3,cb}}


for y = 0, #q-1, 1 do
 for x = 0, #q[1]-1, 1 do
 
  drawRectangle(x*cellw,y*cellh,cellw,cellh,q[y+1][x+1])

 end
end


