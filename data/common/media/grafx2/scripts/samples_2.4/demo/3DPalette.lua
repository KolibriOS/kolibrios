--3D-Palette viewer V0.72 (HSL-models added, 3D-World added, Pen-color only cycles thru unique colors, InputBox)
--by Richard 'Dawnbringer' Fhager

-- Mouse:	Rotate Cube (Stops animation)
-- Arrow-keys:	Move Cube (in 3D world)
-- F1:		Start/Stop animation 
-- F2:		Reset
-- F3:		Increase Color-Size
-- F4:		Decrease Color-Size
-- F5:		(Wip) Cycle thru selected PenColor (Note that only unique colors are displayed)
-- F9:		RGB-space model
--F10:		HSL-space model
--F11:		HSLcubic-space model
-- "+" (Num):	Zoom In
-- "-" (Num):   Zoom Out
-- Esc:		Exit script

-- Drawing updated, rectangle missing, Sep11

run("../libs/dawnbringer_lib.lua")


BRIDIAG_SHOW = 1     -- Show brightness/Grayscale diagonal (1 = on, 0 = off)
ANIM         = 1     -- Animation (1 = on, 0 = off)
BOX_DRK      = 8     -- Darkest   color used for box (0-255)
BOX_BRI      = 112   -- Brightest color used for box (0-255)
COLSIZE_BASE = 26    -- Colors base size (value to adjusted by palette-size, with 2 cols maxsize is v / 1.23)

--
OK,RGB,HSL,HSLC,BOX_BRI,COLSIZE_BASE,SET800x600 = inputbox("3D-Palette Viewer Settings",
                        
                           "1. RGB space [F9]",       1,  0,1,-1,
                           "2. HSL space [F10]",      0,  0,1,-1,
                           "3. HSL-cubic space [F11]",0,  0,1,-1,
                           "Box Brightness (16-255)", BOX_BRI,   16,255,0,  
                           "Col Size (1-100) [F3/F4]", COLSIZE_BASE,   1,100,0,                     
                           "Set Screen to 800x600",      1,0,1,0
                                                                        
);
--

if OK then

 if SET800x600 == 1 then setpicturesize(800,600); end

 SPACE = "rgb"
 FORM  = "cube"
 if HSL == 1 then
  SPACE = "hsl"
  FORM  = "cylinder"
 end
 if HSLC == 1 then
  SPACE = "hsl_cubic"
  FORM  = "cube"
 end


pal = db.fixPalette(db.makePalList(256))

FG = getforecolor() 
BG = getbackcolor()

palcol = FG

--
function initColors(space)
  for n = 1, #pal, 1 do 
   c = pal[n]; 
   if space == "rgb" then
   cols[n] = {c[1]/128-1,c[2]/128-1,c[3]/128-1,c[4]};
  end
  if space == "hsl_cubic" then
   cols[n] = {}
   cols[n][1] = (db.getHUE(c[1],c[2],c[3],0) / 6.0 * 255) / 128 - 1
   cols[n][2] = (db.getSaturation(c[1],c[2],c[3])) / 128 - 1
   cols[n][3] = (db.getLightness(c[1],c[2],c[3])) / 128 - 1
   cols[n][4] = c[4]
  end
  if space == "hsl" then
   cols[n] = {}
   hue = db.getHUE(c[1],c[2],c[3],0) / 6.0 * math.pi*2
   rad = db.getSaturation(c[1],c[2],c[3]) / 256
   cols[n][1] = math.cos(hue) * rad
   cols[n][2] = math.sin(hue) * rad
   cols[n][3] = (db.getLightness(c[1],c[2],c[3])) / 128 - 1
   cols[n][4] = c[4]
  end
 end
end
--

cols = {} -- Make points of palette colors
colz = {} -- To hold calculated points
initColors(SPACE)


function initPointsAndLines(form,bridiag)
 if form == "cube" then 
  pts = {{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1}, -- The box
        {-1,1, 1},{1,1, 1},{1,-1, 1},{-1,-1, 1}}
  lin = {{1,2},{2,3},{3,4},{4,1},{5,6},{6,7},{7,8},{8,5},{1,5},{2,6},{3,7},{4,8}} -- Box Lines
  if bridiag == 1 then lin[13] = {4,6}; end
 end
 if form == "cylinder" then
  p = 28
  pts = {}
  lin = {}
  for n = 1, p, 1 do
   x = math.cos(math.pi*2 / p * (n-1))
   y = math.sin(math.pi*2 / p * (n-1))
   pts[n] = {x,y,-1}
   lin[n] = {n,1 + (n%p)}
   pts[n + p] = {x,y,1}
   lin[n + p] = {n+p,p + 1 + (n%p)}
  end
  lin[p*2+1] = {1,p+1} -- Red (0 degrees)
  lin[p*2+2] = {p+1,p+1+math.ceil(p/2)} -- Lightness end (needs an even # of points to work)
 end
end

boxp = {} -- To hold the calculated points
initPointsAndLines(FORM,BRIDIAG_SHOW)

w,h = getpicturesize()
CX,CY = w/2, h/2



function initAndReset()
 XANG, YANG, ZANG, ZOOM, COLSIZE_ADJ, XD, YD, WORLD_X, WORLD_Y, ZSELECT = 0,0,0,0,0,0,0,0,0,0
end

initAndReset()

SIZE = math.min(w,h)/4
DIST = 5 -- Distance perspective modifier, ~5 is nominal, more means "less 3D" 

CMAXSIZE = math.floor(COLSIZE_BASE / ((#pal)^0.3))
--CMAXSIZE = 8
CMINSIZE = 1 -- Negative values are ok. Color are never smaller than 1 pix

BOX_LINE_DIV = 20 -- Number of colors/segments that a box-line can be divided into (depth)
BOX_DIV_MULT = BOX_LINE_DIV / (math.sqrt(3)*2)

-- Box depth colors
box_div = {}
for n = 0, BOX_LINE_DIV-1, 1 do
 c = BOX_DRK + (BOX_BRI / (BOX_LINE_DIV - 1)) * n 
 --box_div[BOX_LINE_DIV - n] = matchcolor(c,c,c)
 box_div[BOX_LINE_DIV - n] = db.getBestPalMatchHYBRID({c,c,c},pal,0.5,true)
end

--BOX_COL = matchcolor(80,80,80)
BKG_COL = matchcolor(0,0,0)
--CUR_COL = matchcolor(112,112,112)


 function rotate3D(x,y,z,Xsin,Ysin,Zsin,Xcos,Ycos,Zcos) -- PrecCalced cos&sin for speed

   local x1,x2,x3,y1,y2,y3,f,xp,yp

    x1 = x
    y1 = y * Xcos + z * Xsin
    z1 = z * Xcos - y * Xsin

    x2 = x1 * Ycos - z1 * Ysin
    y2 = y1
    z2 = x1 * Ysin + z1 * Ycos

    x3 = x2 * Zcos - y2 * Zsin
    y3 = x2 * Zsin + y2 * Zcos
    z3 = z2

    return x3,y3,z3
  end

 function do3D(x,y,z,zoom,dist,Xsin,Ysin,Zsin,Xcos,Ycos,Zcos) -- PrecCalced cos&sin for speed

   local x1,x2,x3,y1,y2,y3,f,xp,yp

    x1 = x
    y1 = y * Xcos + z * Xsin
    z1 = z * Xcos - y * Xsin

    x2 = x1 * Ycos - z1 * Ysin
    y2 = y1
    z2 = x1 * Ysin + z1 * Ycos

    x3 = x2 * Zcos - y2 * Zsin
    y3 = x2 * Zsin + y2 * Zcos
    z3 = z2

    f = dist/(z3 + dist + zoom) 
    xp = x3 * f
    yp = y3 * f

    return xp,yp,z3
  end


function draw3Dline(x1,y1,z1,x2,y2,z2,div,mult,depthlist)
   local s,xt,yt,xd,yd,zd,xf,yf
   xd = (x2 - x1) / div
   yd = (y2 - y1) / div
   zd = (z2 - z1) / div
   xf,yf = x1,y1

  for s = 1, div, 1 do
    -- Depth assumes a 1-Box (z ranges from -sq(3) to sq(3))
    depth = math.floor(1 + (z1+zd*s + 1.732) * mult) 
    xt = x1 + xd*s -- + math.random()*8
    yt = y1 + yd*s -- + math.random()*8
    c = depthlist[depth]
    if c == null then c = 1; end -- Something isn't perfect, error is super rare but this controls it
    --db.line(xf,yf,xt,yt,c)
    drawline(xf,yf,xt,yt,c)
    xf = xt
    yf = yt
  end
end

function killinertia()
 XD = 0
 YD = 0
end

 -- If using 1-box, z is -sq(3) to sq(3)
 minz = math.sqrt(3)
 totz = minz * 2
 maxrad = CMAXSIZE - CMINSIZE

--q = 0
--delay = 4
--move = 0.03

while 1 < 2 do

 -- Time-for-space-wiggle...or somekindof attempt
 --WORLD_X = -move
 --q = (q + 1) % delay
 --if q < delay/2 then WORLD_X = move; end

 clearpicture(BKG_COL)

 Xsin = math.sin(XANG); Xcos = math.cos(XANG)
 Ysin = math.sin(YANG); Ycos = math.cos(YANG)
 Zsin = math.sin(ZANG); Zcos = math.cos(ZANG) 

 -- Rotate Box points
 for n = 1, #pts, 1 do
  p = pts[n]
  x,y,z = p[1],p[2],p[3]
  XP,YP,zp = rotate3D(x,y,z,Xsin,Ysin,Zsin,Xcos,Ycos,Zcos)
  boxp[n] = {XP,YP,zp}
 end

 -- Rotate Colors in palette
 for n = 1, #cols, 1 do
  p = cols[n]
  x,y,z,c = p[1],p[2],p[3],p[4]
  XP,YP,zp = rotate3D(x,y,z,Xsin,Ysin,Zsin,Xcos,Ycos,Zcos)
  colz[n] = {XP,YP,zp,c} 
 end

 ------------------------------------
 -- Control world
 ------------------------------------

  -- Calculate points anew

 -- Worldize Box points
 for n = 1, #boxp, 1 do
  s = SIZE
  v = boxp[n]
  x = v[1] + WORLD_X
  y = v[2] + WORLD_Y
  z = v[3]
  f = DIST/(z + DIST + ZOOM)
  XP = CX + x * f * s 
  YP = CY + y * f * s
  boxp[n] = {XP,YP,z}
 end

 -- Worldize Colors in palette
 for n = 1, #colz, 1 do
  s = SIZE
  v = colz[n]
  x = v[1] + WORLD_X
  y = v[2] + WORLD_Y
  z = v[3]
  c = v[4]
  f = DIST/(z + DIST + ZOOM)
  XP = CX + x * f * s 
  YP = CY + y * f * s
  colz[n] = {XP,YP,z,c} 
 end


-------------------------------------
-------------------------------------

 -- Brightness Diagonal
 --if BRIDIAG_SHOW == 1 then
 -- p1 = boxp[4]
 -- p2 = boxp[6]
 -- x1,y1,z1 = p1[1],p1[2],p1[3]
 -- x2,y2,z2 = p2[1],p2[2],p2[3]
 -- draw3Dline(x1,y1,z1,x2,y2,z2,BOX_LINE_DIV,BOX_DIV_MULT,box_div)
 --end

  --c1 = math.min(FG,BG)
  --c2 = math.max(FG,BG)
  --p = colz[26]
  --XP1,YP1,zp1,c1 = p[1],p[2],p[3],p[4]
  --for n = #colz, 1, -1 do
  -- p = colz[27]
  -- XP2,YP2,zp2,c2 = p[1],p[2],p[3],p[4]
  -- drawline(XP1,YP1,XP2,YP2,c1)
  --end

 -- sort on z
 db.sorti(colz,3) 

 -- Draw colors
 for n = #colz, 1, -1 do
  p = colz[n]
  XP,YP,zp,c = p[1],p[2],p[3],p[4]

  radius = CMINSIZE + maxrad - (zp+minz) / totz * maxrad
  dorad = math.floor(radius - ZOOM*2 +  COLSIZE_ADJ) 

  if dorad >= 1 then
   --db.drawCircle(XP,YP,dorad,c) 
   drawdisk(XP,YP,dorad,c) 
   --db.drawRectangle(XP,YP,dorad,dorad,c)
   else putpicturepixel(XP,YP,c)
  end

  if c == FG or c == BG then
   sz = math.max(3,dorad + 3)
   if c == BKG_COL then v = (c+128) % 255; c = matchcolor(v,v,v); end
   db.drawRectangleLine(XP-sz,YP-sz,sz*2,sz*2,c)
  end

 end -- colz



 -- Draw box
 for n = 1, #lin, 1 do
  
  l = lin[n]
  p1 = boxp[l[1]]
  p2 = boxp[l[2]]
  x1,y1,z1 = p1[1],p1[2],p1[3]
  x2,y2,z2 = p2[1],p2[2],p2[3]
  draw3Dline(x1,y1,z1,x2,y2,z2,BOX_LINE_DIV,BOX_DIV_MULT,box_div)

 end -- eof box

 --updatescreen(); if (waitbreak(0.00)==1) then return; end

 repeat

    old_key = key;
    old_mouse_x = mouse_x;
    old_mouse_y = mouse_y;
    old_mouse_b = mouse_b;
    
    updatescreen()
  
    moved, key, mouse_x, mouse_y, mouse_b = waitinput(0)
    
    if mouse_b == 1 then ANIM = 0; end

    if (key==27) then
       return;
    end

    if (key==282) then  ANIM = (ANIM+1) % 2;  end -- F1: Stop/Start Animation
    if (key==283) then  initAndReset();       end -- F2: Reset all values
    if (key==284) then  COLSIZE_ADJ = COLSIZE_ADJ + 0.5;       end -- F3
    if (key==285) then  COLSIZE_ADJ = COLSIZE_ADJ - 0.5;       end -- F4

   --messagebox(key)

    if (key==286) then  
      --FG = (FG + 1) % 255; 
      palcol = (palcol + 1) % #pal 
      FG = pal[palcol+1][4]
      setforecolor(FG);
      setcolor(0,getcolor(0))  -- Force update of palette until setforecolor() is fixed
     end -- F5

    if (key==290) then -- F9
     initColors("rgb")
     initPointsAndLines("cube",BRIDIAG_SHOW)
    end
    if (key==291) then -- F10
     initColors("hsl")
     initPointsAndLines("cylinder", 0) -- Bridiag won't show even if turned on, it's only for cube
    end
    if (key==292) then -- F11
     initColors("hsl_cubic")
     initPointsAndLines("cube",BRIDIAG_SHOW)
    end

    if (key==269) then  ZOOM = ZOOM + 0.1;  end
    if (key==270) then  ZOOM = ZOOM - 0.1;  end

    if (key==32) then  
       ZSELECT = (ZSELECT + math.pi/2) % (2*math.pi);  
       --YANG = ((YANG - math.pi/2) % (math.pi*2));
       --XANG = ((XANG + math.pi/2) % (math.pi*2));
    

      YANG = ((YANG + math.pi/2) % (math.pi*2));
      XANG = ((XANG + math.pi/2) % (math.pi*2));
      YANG = ((YANG - math.pi/2) % (math.pi*2));
    end -- Rotate Z 90 Degrees

    SPEED = math.pi / 100
 

    if (key==273) then WORLD_Y = WORLD_Y - 0.05; killinertia();  end
    if (key==274) then WORLD_Y = WORLD_Y + 0.05; killinertia();  end

    if (key==276) then WORLD_X = WORLD_X - 0.05; killinertia();  end
    if (key==275) then WORLD_X = WORLD_X + 0.05; killinertia();  end
   
  until ((mouse_b == 1 and (old_mouse_x~=mouse_x or old_mouse_y~=mouse_y)) or key~=0 or ANIM==1 or math.abs(XD)>0.01 or math.abs(YD)>0.01);

 if ANIM == 0 then
  if (mouse_b==1 and (old_mouse_x~=mouse_x or old_mouse_y~=mouse_y)) then -- Inertia
   XD = (mouse_y - old_mouse_y)*0.005
   YD = (mouse_x - old_mouse_x)*0.005
    else
     XD = XD*0.92
     YD = YD*0.92
  end
  XANG = ((XANG - XD) % (math.pi*2));
  YANG = ((YANG + YD) % (math.pi*2));
  ZANG = ZSELECT
 end 

 if ANIM == 1 then
    XANG = (XANG + math.pi/300) % (math.pi*2)
    YANG = (YANG + math.pi/500) % (math.pi*2)
    ZANG = (ZANG + math.pi/1000) % (math.pi*2)
 end

 --XANG = ((CY-mouse_y) / 200  % (math.pi*2));
  --YANG = ((mouse_x - CX) / 200  % (math.pi*2));
  --ZANG = 0

 statusmessage("x"..math.floor(XANG*57.3).."° y"..math.floor(YANG*57.3).."° z"..math.floor(ZANG*57.3).."° Zm: "..math.floor(-ZOOM*10).."   ")

end

end -- OK
