View3ds 0.071 - tiny viewer to .3ds and .asc files with several graphics
                effects implementation.


What's new?
1. New displaying model - glass -  it's two pass rendering. First pass calculates
   Z position of all front pixels, second render image with adding reflective
   component of light only for front pixels. Transparent effect by adding with saturation.
2. I removed bug with performing generation object after choosing 'emboss' option.

Buttons description:
1.  rotary: choosing rotary axle: x, y, x+y.
2.  shd. model: choosing shading model: flat, grd (smooth), env (spherical
    environment mapping, bump (bump mapping), tex (texture mapping),
    pos (position shading depend), dots (app draws only points - nodes of object),
    txgrd (texture mapping + smooth shading),  2tex (texture mapping + spherical
    environment mapping), bmap (bump + texture mapping),  cenv (cubic environment
    mapping), grdl (Gouraud lines - edges only), rphg (real Phong).
3.  speed: idle, full.
4,5. zoom in, out: no comment.
6.  catmull: disabled
7.  culling: backface culling on/ off.
8.  rand. light: Randomize 3 unlinear lights( so called Phong's illumination).
9.  Blur: blur N times; N=0,1,2,3,4,5
10.11,12,13. loseless operations (rotary 90, 180 degrees).
12. emboss: Do emboss effect( flat bumps ), use 'bumps deep' button to do edges more deep.
13. fire: do motion blur ( looks like fire ).
14. move: changes meaning x,y,z +/- buttons  ->  obj: moving object, camr: moving camera, wave: x,y +/- increase,
    decrease wave effect frequency and amplitude.
15. generate: Generates some objects: node, Thorn Crown, heart...
16. bumps: random, according to texture.
17. bumps deep -> create bumps deeper or lighter.
18. re-map tex -> re-map texture and bump map coordinates, to change spherical mapping
    around axle use 'xchg' and 'mirror' buttons, then press 're-map tex' button.
19. bright + -> increase picture brightness.
20. bright - -> decrease picture brightness.
21. wav effect -> do effect based sine function.
22. editor -> setting editing option. If is "on" then red bars are draw according to each vertex, Pressing
    and moving left mouse button (cursor must be on handler)- change vertex position. If left mouse button
    is released apply current position. You may also decrease whole handlers count by enable culling (using
    appropriate button) - some back handlers become hidden.

                         Maciej Guba             VIII 2020
