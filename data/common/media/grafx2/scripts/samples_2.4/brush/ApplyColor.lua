--BRUSH Remap: Apply PenColor
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

run("../libs/dawnbringer_lib.lua")

OK,tin,clz,fade,amt,brikeep,falloff,nobg,nopen,briweight   = inputbox("Apply PenColor 2 Brush",
                        
                           "1. Tint",               1,  0,1,-1,
                           "2. Colorize",           0,  0,1,-1,
                           "BG->FG color Fade", 0,  0,1,0, 
                           "AMOUNT % (0-100)", 100,  0,100,0,  
                           "Preserve Brightness", 1,  0,1,0,
                           "Bri/Dark FallOff", 1,  0,1,0,
                           "Exclude Background",     1,0,1,0,
                           "Exclude PenColor",       0,0,1,0,
                           "ColMatch Bri-Weight %", 25,  0,100,0                                                     
);


if OK == true then

 function cap(v) return math.min(255,math.max(v,0)); end

 w, h = getbrushsize()


 fg = getforecolor()
 bg = getbackcolor()
 fR,fG,fB = getcolor(fg)
 bR,bG,bB = getcolor(bg)

 pal = db.fixPalette(db.makePalList(256))
 if nobg == 1 then
  pal = db.stripIndexFromPalList(pal,bg) -- Remove background color from pallist
 end
 if nopen == 1 then
  pal = db.stripIndexFromPalList(pal,fg) -- Remove  Pencolor from pallist
 end


 amtA = amt / 100
 amtR = 1 - amtA

  -- Normalize Pen Color
  lev = (fR+fG+fB)/3
  fR = fR - lev
  fG = fG - lev
  fB = fB - lev

 ---------------------------------------------------
 -- Colorize (Colourant) (just apply colorbalance)
 -- Tint (make grayscale and apply colorbalance)
 --
 -- I think it should be the other way around since colorize is the process of adding color to B&W film...
 -- But this is the what Brilliance and others call it 
 --
 if clz == 1 or tin == 1 then
  cols = {}
  for n = 0, 255, 1 do

  r,g,b = getcolor(n)
  a = db.getBrightness(r,g,b)

    
  mR,mG,mB = fR,fG,fB

  -- Fade between bg & fg pencolor across dark-bright
  if fade == 1 then
    lf = a / 255
    lr = 1 - lf
    mR = bR*lr + fR*lf
    mG = bG*lr + fG*lf
    mB = bB*lr + fB*lf
    lev = (mR+mG+mB)/3
    mR = mR - lev
    mG = mG - lev
    mB = mB - lev
  end   

  fr,fg,fb = mR,mG,mB
  

     if brikeep == 1 then
       -- Loose Brightness preservation (ex: applying full red to dark colors)
       brin = db.getBrightness(cap(r+mR),cap(g+mG),cap(b+mB))
       itot = brin - a
       fr = mR - itot 
       fg = mG - itot
       fb = mB - itot
     end 

     -- Falloff (Effect weakens at dark and bright colors)
     if falloff == 1 then
      fo =  1 - math.abs((a - 127.5)/127.5)^2
      fr = fr * fo
      fg = fg * fo
      fb = fb * fo
     end

     if tin == 1 then
      --cols[n+1] = matchcolor((a+fr)*amtA + r*amtR, (a+fg)*amtA + g*amtR, (a+fb)*amtA + b*amtR)
      cols[n+1] = db.getBestPalMatchHYBRID({(a+fr)*amtA+r*amtR, (a+fg)*amtA + g*amtR, (a+fb)*amtA + b*amtR},pal,briweight / 100,true)
     end
     if clz == 1 then
      --cols[n+1] = matchcolor((r+fr)*amtA + r*amtR, (g+fg)*amtA + g*amtR, (b+fb)*amtA + b*amtR)
      cols[n+1] = db.getBestPalMatchHYBRID({(r+fr)*amtA+r*amtR, (g+fg)*amtA + g*amtR, (b+fb)*amtA + b*amtR},pal,briweight / 100,true)
     end
  end

  if nobg == 1 then cols[getbackcolor()+1] = getbackcolor(); end

  for x = 0, w - 1, 1 do
   for y = 0, h - 1, 1 do
    putbrushpixel(x, y, cols[getbrushpixel(x,y) + 1]);
  end
 end
end; 
-- eof Colorize & Tint
--------------------------------------------------------

end -- OK
