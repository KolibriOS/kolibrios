include 'me_skin.inc'

SKIN_PARAMS \
  height          = bmp_base.height,\     
  margins         = [5:1:97:1],\          
  colors active   = [binner=0xf3f3f3:\    
                     bouter=0xffffff:\    
                     bframe=0xf3f3f3],\   
  colors inactive = [binner=0xf3f3f3:\    
                     bouter=0xffffff:\    
                     bframe=0xf3f3f3],\   
  dtp             = 'Win10.dtp'          

SKIN_BUTTONS \
  close    = [-47:0][45:21],\             
  minimize = [-92:0][45:21]               

SKIN_BITMAPS \
  left active   = bmp_left,\              
  left inactive = bmp_left1,\
  oper active   = bmp_oper,\
  oper inactive = bmp_oper1,\
  base active   = bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'left.bmp'               
BITMAP bmp_oper ,'oper.bmp'
BITMAP bmp_base ,'base.bmp'
BITMAP bmp_left1,'left_1.bmp'
BITMAP bmp_oper1,'oper_1.bmp'
BITMAP bmp_base1,'base_1.bmp'