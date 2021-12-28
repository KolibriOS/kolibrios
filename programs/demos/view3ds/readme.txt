View3ds 0.076 - tiny viewer to .3ds and .asc files with several graphics
                effects implementation.

What's new?
1. Detecting manifold chunks procedure based on kind of sorted pivot 
    table. Chunks are counted and this number displayed.
2. New calculating normal vectors proc that use some data produced
    by new chunks routine. Now big object loading is fast. I load object that 
    contains ~500000 vertices,  ~700000 faces and  ~2000 0000 unique edges
    in few seconds on i5 2cond gen. Earlier such objects calculating was
    rather above time limits.
3. On http://board.flatassembler.net occasionaly there are some disccusions
    about optimizing. Some clever people, wich skills and competence I trust,
    claims - for CPU's manufactured last  ~15 years size of code is crucial 
    for speed. (Better utilize CPU cache).
    So I wrote some 'movsd' mnemonics instead  'mov [edi],sth'; 'loop' instead
    'dec ecx,jnz sth'. Moreover I come back to init some local varibles 
    by 'push' (flat_cat.inc). I took effort to change divisions to 
    multiplications  two_tex.inc  (works ok in fpu only Ext = NON mode and
    of course in Ext = SSE3 mode),  grd_tex.inc (single line not parallel 
    muls, whole drawing routine  4 divs instead 27 divisions), 
    bump_tex.inc - 3 divs in SSE2 mode.s  See sources for details. 
4. Editor button allows now editing by vertex all above 65535 vert objects.

	


Buttons description:
1.  rotary: choosing rotary axle: x, y, x+y, keys - for object translate
    using keyboard.	 .
2.  shd. model: choosing shading model: flat, grd (smooth), env (spherical
    environment mapping, bump (bump mapping), tex (texture mapping),
    pos (position shading depend), dots (app draws only points - nodes of object),
    txgrd (texture mapping + smooth shading),  2tex (texture mapping + spherical
    environment mapping), bmap (bump + texture mapping),  cenv (cubic environment
    mapping), grdl (Gouraud lines - edges only), rphg (real Phong), glas (glass effect),
    ptex (real Phong + texturing + transparency).
3.  speed: idle, full.
4,5. zoom in, out: no comment.
6.  ray shadow: calc ray casted shadows.
7.  culling: backface culling on/ off.
8.  rand. light: Randomize 3 unlinear lights( so called Phong's illumination).
9.  Blur: blur N times; N=0,1,2,3,4,5
10.11,12,13. loseless operations (rotary 90, 180 degrees).
12. emboss: Do emboss effect( flat bumps ), use 'bumps deep' button to do edges
     more deep.
13. fire: do motion blur ( looks like fire ).
14. move: changes meaning x,y,z +/- buttons  ->  obj: moving object, camr: moving
      camera, wave: x,y +/- increase, decrease wave effect frequency and amplitude.
15. generate: Generates some objects: node, Thorn Crown, heart...
16. bumps: random, according to texture.
17. bumps deep -> create bumps deeper or lighter.
18. re-map tex -> re-map texture and bump map coordinates, to change spherical
     mapping  around axle use 'xchg' and 'mirror' buttons, then press 're-map tex' button.
19. bright + -> increase picture brightness.
20. bright - -> decrease picture brightness.
21. wav effect -> do effect based sine function.
22. editor -> setting editing option. If is "on" then red bars are draw according to each
   vertex, Pressing  and moving left mouse button (cursor must be on handler)- change
   vertex position. If left mouse button is released apply current position. You may also
   decrease whole handlers count by enable culling (using appropriate button) - some
   back handlers become hidden.

                         Maciej Guba             XII 2021
