--PICTURE scene: Ellipse update-demo (anim)
--Demonstrates 'interactive' features.
--by Richard Fhager

-- Copyright 2011 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>


--
-- rot: Rotation in degrees
-- stp: Step is # of line segments (more is "better")
-- a & b are axis-radius
function ellipse2(x,y,a,b,stp,rot,col)
 local n,m=math,rad,al,sa,ca,sb,cb,ox,oy,x1,y1,ast
 m = math; rad = m.pi/180; ast = rad * 360/stp;
 sb = m.sin(-rot * rad); cb = m.cos(-rot * rad)
 for n = 0, stp, 1 do
  ox = x1; oy = y1;
  sa = m.sin(ast*n) * b; ca = m.cos(ast*n) * a
  x1 = x + ca * cb - sa * sb
  y1 = y + ca * sb + sa * cb
  if (n > 0) then drawline(ox,oy,x1,y1,col); end
 end
end
--

setpicturesize(300,300)
setcolor(0,96,96,96)
setcolor(1,255,255,128)

r1 = 100
r2 = 50
rt = 0

frames = 100


while (1 < 2) do

 r1t = 10 + math.random() * 140
 r2t = 10 + math.random() * 140
 rtt = math.random() * 360

for n = 0, frames-1, 1 do
 clearpicture(0)

 f2 = n / frames
 f1 = 1 - f2

 r1a = r1*f1 + r1t*f2
 r2a = r2*f1 + r2t*f2
 rta = rt*f1 + rtt*f2

 --       x,   y,   r1,  r2,  stp, rot, col
 ellipse2(150, 150, r1a, r2a, 50,  rta, 1)

  statusmessage('press ESC to stop')
  updatescreen();if (waitbreak(0)==1) then return end

end

 r1,r2,rt = r1a,r2a,rta

end
