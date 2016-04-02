non_rot_light:
;light:
dd 0.9, 0.9, 1.0
;light2:
dd 0.1, 0.05, 0.5
;light3:
dd 0.9, 0.1, 0.3


;sph_max     dd 2
non_rot_sphere:
 dd 0.7
 dd 0.3
 dd 0.4   ;,  0.0

 dd 0.2
 dd 0.7
 dd 0.3   ;,  0.0

 dd 0.1
 dd 0.1
 dd 0.1   ;,  0.0

 dd 0.7
 dd 0.7
 dd 0.1  ;,  0.0

 dd 0.5
 dd 0.2
 dd 0.1  ;,  0.0


sph_radius:
R dd 0.25
R2 dd 0.22
r3 dd 0.1
r4 dd 0.25
r5 dd 0.1

align 16
camera:
x0 dd  0.5
y0 dd  0.5
z0 dd  - 12.0  , 0.0

;x1 dd
;y1 dd
;z1 dd
float2:
dd 2.0, 2.0, 2.0, 2.0
float4:
dd 4.0, 4.0, 4.0, 4.0
float255:
dd 255.0, 255.0, 255.0, 255.0
;correct_tex:
;dd 64.0, 64.0, 64.0, 64.0
;light:
;dd 0.9, 0.9, 1.0, 0.0
;light2:
;dd 0.1, 0.05, 0.5, 0.0
;light3:
;dd 0.9, 0.1, 0.3, 0.0
lights_color:
dd 250.0, 255.0, 255.0, 0.0
dd 232.0, 254.0, 220.0, 0.0
dd 225.0,37.0, 247.0, 0.0

light_factor dd 0.33
tex_x dd 128
align 16
nearest_sphere:
rd 4
nearest_radius dd ?
smalest_t dd ?
align 16
light:
  rd 4 * MAX_LIGHTS
sphere:
  rd 4 * MAX_SPHERES
;bitmap file '128x128.raw'