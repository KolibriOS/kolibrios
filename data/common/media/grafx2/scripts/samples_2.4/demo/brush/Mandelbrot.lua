--BRUSH Scene: Mandelbrot fractal v0.5
--
--Draws a Mandelbrot fractal in the current brush.
--
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

-- This script was adopted from Evalion, a Javascript codecrafting/imageprocessing project
--http://goto.glocalnet.net/richard_fhager/evalion/evalion.html 


colors = 64

x0   = -1.7
x1   =  0.7
ym   =  0
iter =  64


ok, x0, x1, ym, iter = inputbox("Fractal data",
  "X0",   x0,   -2, 2,4,
  "X1",   x1,   -2, 2,4,
  "midY", ym,   -2, 2,4,
  "Iter", iter, 1, 2048,0
);

-- -0.831116819,-0.831116815,0.2292112435,192


function mandel(x,y,l,r,o,i) -- pos. as fraction of 1, left coord, right coord, y coord, iterations 

  local w,s,a,p,q,n,v,w

  s=math.abs(r-l);

  a = l + s*x;
  p = a;
  b = o - s*(y-0.5);
  q = b;
  n = 1;
  v = 0;
  w = 0; 

  while (v+w<4 and n<i) do n=n+1; v=p*p; w=q*q; q=2*p*q+b; p=v-w+a; end;

  return n
end


w, h = getbrushsize()

for x = 0, w - 1, 1 do
  for y = 0, h - 1, 1 do
    q = mandel(x/w,y/h,x0,x1,ym,iter) % colors;
    putbrushpixel(x, y, q);
  end
end
