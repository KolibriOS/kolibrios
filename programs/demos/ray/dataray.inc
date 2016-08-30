
non_rot_light:
;light:
dd 0.5, 0.5, 1.9
;light2:
dd 1.1, 1.05, 0.5
;light3:
dd 1.9, 0.1, 3.3

dd 0.5, 0.5, -4.0
;light2:
dd 0.2, 0.2, -5.0
;light3:
dd 0.1, 0.9, -3.93

;sph_max     dd 2
non_rot_sphere:
 dd 0.05
 dd 0.05
 dd 0.05   ;,  0.0

 dd 0.7
 dd 0.7
 dd 0.7   ;,  0.0

 dd 0.5
 dd 0.5
 dd 0.41   ;,  0.0

 dd 0.7
 dd 0.7
 dd -4.1  ;,  0.0

 dd 0.5
 dd 0.2
 dd 0.1  ;,  0.0



 dd 0.3
 dd 0.2
 dd 0.1   ;,  0.0

 dd 0.4
 dd 0.7
 dd 0.5  ;,  0.0

 dd 0.05
 dd 0.25
 dd 0.8  ;,  0.0

sph_radius:
R dd 0.09
R2 dd 0.22
r3 dd 0.1
r4 dd 0.25
r5 dd 0.1

r6 dd 0.19
r7 dd 0.1
   dd 0.17


align 16
surface:
dd 0.1, -1.0, 0.0  ; A B C constans in Ax+BY+Cz+D=0 surface equation
		  ; make sure if normalized
dd 0.4		  ; D element of equation above

camera:
x0 dd  0.5
y0 dd  0.5
z0 dd  - 6.0  , 0.0

;x1 dd
;y1 dd
;z1 dd
float2:
dd 2.0, 2.0, 2.0, 2.0
float4:
dd 4.0, 4.0, 4.0, 4.0
float255:
dd 255.0, 255.0, 255.0, 255.0
zero:
dd 0.0, 0.0, 0.0, 0.0
;correct_tex:
;dd 64.0, 64.0, 64.0, 64.0
;light:
;dd 0.9, 0.9, 1.0, 0.0
;light2:
;dd 0.1, 0.05, 0.5, 0.0
;light3:
;dd 0.9, 0.1, 0.3, 0.0
lights_color:
dd 250.0, 255.0, 155.0, 0.0
dd 200.0, 254.0, 0.0, 0.0
dd 255.0,37.0, 247.0, 0.0

dd 250.0, 255.0, 255.0, 0.0
dd 120.0, 54.0, 199.0, 0.0
dd 25.0,137.0, 247.0, 0.0
shadow_factor:
 dd 50.0, 50.0, 50.0, 50.0
positive_mask:
 dd 0x7fffffff, 0x7fffffff, 0x7fffffff, 0.0
ambient_col:
db 30, 20, 25, 20
;light_factor dd  0.33
;tex_x dd 128
align 16
nearest_sphere:
rd 4
nearest_radius dd ?
smalest_t dd ?
align 16
surface_without_d:
  rd 4
light:
  rd 4 * MAX_LIGHTS
sphere:
  rd 4 * MAX_SPHERES
;bitmap file '128x128.raw'