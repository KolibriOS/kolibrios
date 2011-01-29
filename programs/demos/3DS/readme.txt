View3ds 0.054 - tiny viewer to .3ds files.

What's new?
1. Skinned window by Leency.
2. Optimizations.
3. Re map texture, bumps option - allow spherical mapping around each axle (X,Y,Z).
4. Problem with too small memory to generate object fixed. (Problem ocurred with
   house.3ds object and others objects contains less than 1000 faces and points).

Buttons description:
1. rotary: choosing rotary axle: x, y, x+y.
2. shd. model: choosing shading model: flat, grd (smooth), env (spherical
   environment mapping, bump (bump mapping), tex (texture mapping),
   pos (position shading depend), dots (app draws only points - nodes of object),
   txgrd (texture mapping + smooth shading),  2tex (texture mapping + spherical
   environment mapping), bmap (bump + texture mapping),  cenv (cubic environment
   mapping).
3. speed: idle, full.
4,5. zoom in, out: no comment.
6. catmull: on( use z buffer ( z coordinate interpolation), off( depth sorting, painters
   alghoritm).Txgrd and 2tex models only with catmull  = on.
7. culling: backface culling on/ off.
8. rand. light: Randomize 3 unlinear lights( so called Phong's illumination).
9. Blur: blur N times; N=0,1,2,3,4,5
10.11,12,13. loseless operations (rotary 90, 180 degrees).
12. emboss: Do emboss effect( flat bumps ), use blur to do edges more deep.
       carefull with emboss + fire - it looks annoying.
13. fire: do movement blur ( looks like fire ).
14. move: changes meaning x,y,z +/- buttons  ->  obj: moving object, camr: moving camera.
15. generate: Generates some objects: node, Thorn Crown, heart...
16. bumps: random, according to texture.
17. bumps deep -> create bumps deeper or lighter.
18. re-map tex -> re-map texture and bump map coordinates, to change spherical mapping 
    around axle use 'xchg' and 'mirror' buttons, then press 're-map tex' button.

			 Macgub 		december 2009
                                                                                  Last edited Jan 2010