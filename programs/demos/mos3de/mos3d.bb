AppTitle "3D Cube Demo","hasta la vista"
While f$<>"1" And f$<>"2" 
 f$=Input("Fullscreen (1) or Windowed? (2)") 
 If f$="1"
  Graphics 320,240,16,1
 EndIf
 If f$="2"
  Graphics 320,240,16,2
 EndIf
Wend

SetBuffer BackBuffer()

; -- init texturemapper
texturefile$="wall3.bmp"
If FileType(texturefile$)=1 Then ; if it exists then load it...
 txt=LoadImage(texturefile$); use any 256*256 pixel texture
Else
 txt=CreateImage(256,256)   ; else create one on the fly
 SetBuffer ImageBuffer(txt)
 For i=0 To 255
  Color i,255-i,0
  Line 255-i,0,255,i
  Color 255,255-i,255-i
  Line 0,255-i,i,255
 Next
 SetBuffer BackBuffer()
EndIf
Global imgtxt=ImageBuffer(txt)

Dim Lefttable%(480, 2), Righttable%(480, 2)  ;Scan converter tables (make shure to reserve enough)
Dim Polypoints%(3, 1) ; Array for polygon co-ords, 4 pairs(x,y) co -ords
Global Miny%, Maxy%
Global Pwidth%, Pheight%
 
 
Pwidth = 63 Shl 16   ;original picture width in pixels -1 shl 16
Pheight = 63 Shl 16 ;original picture height in pixels -1 shl 16

; eo init texturemapper

Dim zbuffer(10000) ; maximum of 10k Quads (don't worry, would be too slow anyway :) )
 
; Read in a mesh
Restore building
Read anz
Dim xwww(anz),ywww(anz),zwww(anz)
Dim xw(anz),yw(anz),zw(anz)
 
For i=0 To anz
 Read xwww(i)
 Read ywww(i)
 Read zwww(i)
Next

a=0

alpha=1
beta=1
gamma=1

zoom=-500

; MMMMMMMMMMMMMMMMMMMMMMMMMmmmmmain

While KeyDown(1)=0

 Miny% = 32767
 Maxy% = 0

 Color 0,0,0
 Rect 0,0,320,240,1

 a=a+1.0 ; automatic rotation...
 If a>359.9 Then a=0
 alpha=a
 beta=a
 gamma=a+a Mod 360

 mausy#=0.1+(MouseY()/50.0)
 ;mausy#=2.0
 For i=0 To anz
  xl1#=zwww(i)*Sin(gamma)+xwww(i)*Cos(gamma)
  yl1#=ywww(i)
  zl1#=zwww(i)*Cos(gamma)-xwww(i)*Sin(gamma)

  xl2#=xl1
  yl2#=yl1*Cos(beta)-zl1*Sin(beta)
  zl2#=yl1*Sin(beta)+zl1*Cos(beta)
 
  xl3#=(yl2*Sin(alpha)+xl2*Cos(alpha))
  yl3#=(yl2*Cos(alpha)-xl2*Sin(alpha))
  zl3#=(zl2)

;  xl3#=xwww(i)
;  yl3#=ywww(i)
;  zl3#=zwww(i)

  If yloc# - Zoom <> 0 Then yloc = Int(yl3 ) * 200 / (zl3 - Zoom)
  If xloc# - Zoom <> 0 Then xloc = Int(xl3 ) * 200 / (zl3 - Zoom)

  xw(i)=((mausy#)*xloc) +160
  yw(i)=((mausy#)*yloc) +120
  zw(i)=(zl3+256)
 Next

 ; z-sorting...
 For i=0 To 10000
  zbuffer(i)=-1 
 Next
 For i=0 To anz-3 Step 4
  If zw(i)>=0 ; clip Quads behind Camera
   zwmax=zw(i)
   If zwmax<zw(i+1) Then zwmax=zw(i+1)
   If zwmax<zw(i+2) Then zwmax=zw(i+2)
   If zwmax<zw(i+3) Then zwmax=zw(i+3)
   While zbuffer(zwmax)<>-1 And zwmax<10000
    zwmax=zwmax+1
   Wend
   zbuffer(zwmax)=i
  EndIf
 Next

 LockBuffer ImageBuffer(txt)
 LockBuffer BackBuffer()

 For i2=10000 To 0 Step -1 ; reading quads in z-order from far to near
  i=zbuffer(i2)

  If i>-1 And i< anz-2 ; if it isn't -1 then it's a Quad Point 1 ID
   ; Mapping...
   GetPolygonPoints(i)
   FindSmallLargeY()
   X1% = Polypoints%(0, 0)
   Y1% = Polypoints%(0, 1)
   X2% = Polypoints%(1, 0)
   Y2% = Polypoints%(1, 1)
   ScanConvert(X1%, Y1%, X2%, Y2%, 1)     ;scan top of picture
   X1% = Polypoints%(1, 0)
   Y1% = Polypoints%(1, 1)
   X2% = Polypoints%(2, 0)
   Y2% = Polypoints%(2, 1)
   ScanConvert(X1%, Y1%, X2%, Y2%, 2)   ;scan Right of picture
   X1% = Polypoints%(2, 0)
   Y1% = Polypoints%(2, 1)
   X2% = Polypoints%(3, 0)
   Y2% = Polypoints%(3, 1)
   ScanConvert(X1%, Y1%, X2%, Y2%, 3)  ;scan bottom of picture
   X1% = Polypoints%(3, 0)
   Y1% = Polypoints%(3, 1)
   X2% = Polypoints%(0, 0)
   Y2% = Polypoints%(0, 1)
   ScanConvert(X1%, Y1%, X2%, Y2%, 4)    ;scan Left of picture

   TextureMap()
  EndIf
 Next

 UnlockBuffer BackBuffer()
 UnlockBuffer ImageBuffer(txt)

 Color 0,255,0
 Text 20,20,"Move Mouse"

 Flip 0
Wend

End





 
; --- texture mapping functions

Function GetPolygonPoints(ilocal%) ; initially read in a rectangle
 For Count% = 0 To 3
  Polypoints%(Count%, 0) = xw(ilocal%+Count%)
  Polypoints%(Count%, 1) = yw(ilocal%+Count%)
 Next
End Function

Function FindSmallLargeY()
 For Count% = 0 To 3
  Ycoord% = Polypoints%(Count%, 1)
  If Ycoord% < Miny% Then       ; is this the New lowest y co-ord?
   Miny% = Ycoord%             ; Yes...
  End If
  If Ycoord% > Maxy% Then       ; is this the New highest y co-ord?
   Maxy% = Ycoord%             ; Yes...
  End If
 Next
End Function

Function ScanConvert (X1%, Y1%, X2%, Y2%, Pside)
 If Y2% < Y1% Then
  temp%=X1% : X1%=X2% : X2%=temp%
  temp%=Y1% : Y1%=Y2% : Y2%=temp%
  Lineheight% = (Y2% - Y1%)
  ScanLeftSide(X1%, X2%, Y1%, Lineheight%, Pside)
 Else
  Lineheight% = (Y2% - Y1%)
  ScanRightSide(X1%, X2%, Y1%, Lineheight%, Pside)
 End If
End Function

Function ScanLeftSide (X1%, X2%, Ytop%, Lineheight%, Pside)
 Lineheight% = Lineheight% + 1       ; prevent divide by zero
 Xadd = (X2% - X1%) Shl 16
 Xadd = Xadd / Lineheight%

 
 If Pside = 1 Then
  Px = Pwidth% - 1
  Py = 0
  Pxadd = -Pwidth%  / Lineheight%
  Pyadd = 0
 End If
 If Pside = 2 Then
  Px = Pwidth%
  Py = Pheight%
  Pxadd = 0
  Pyadd = -Pheight%  / Lineheight%
 End If
 If Pside = 3 Then
  Px = 0
  Py = Pheight%
  Pxadd = Pwidth%  / Lineheight%
  Pyadd = 0
 End If
 If Pside = 4 Then
  Px = 0
  Py = 0
  Pxadd = 0
  Pyadd = Pheight%  / Lineheight%
 End If

 x = X1% Shl 16
 For y% = 0 To Lineheight%
  Ytopy%=Ytop%+y%
  If Ytopy%<0 Then Ytopy%=0
  Lefttable(Ytopy%, 0) = x Sar 16    ;polygon x
  Lefttable(Ytopy%, 1) = Px          ;picture x
  Lefttable(Ytopy%, 2) = Py          ;picture y
  x = x + Xadd                       ;Next polygon x
  Px = Px + Pxadd                    ;Next picture x
  Py = Py + Pyadd                    ;Next picture y
 Next
End Function

Function ScanRightSide (X1%, X2%, Ytop%, Lineheight%, Pside)
 Lineheight% = Lineheight% + 1    ; No divide by zero
 Xadd = (X2% - X1%) Shl 16
 Xadd = Xadd / Lineheight%

 If Pside = 1 Then
  Px = 0
  Py = 0
  Pxadd = Pwidth% / Lineheight%
  Pyadd = 0
 End If
 If Pside = 2 Then
  Px = Pwidth%
  Py = 0
  Pxadd = 0
  Pyadd = Pheight% / Lineheight%
 End If
 If Pside = 3 Then
  Px = Pwidth%
  Py = Pheight%
  Pxadd = -Pwidth% / Lineheight%
  Pyadd = 0
 End If
 If Pside = 4 Then
  Px = 0
  Py = Pheight%
  Pxadd = 0
  Pyadd = -Pheight% / Lineheight%
 End If

 x = X1% Shl 16
 For y% = 0 To Lineheight%
  Ytopy%=Ytop%+y%
  If Ytopy%<0 Then Ytopy%=0
  Righttable(Ytopy%, 0) = x Sar 16   ;polygon x
  Righttable(Ytopy%, 1) = Px         ;picture x
  Righttable(Ytopy%, 2) = Py         ;picture y
  x = x + Xadd                       ;Next polygon x
  Px = Px + Pxadd                    ;Next picture x
  Py = Py + Pyadd                    ;Next picture y
 Next 
End Function





Function TextureMap()
 For y% = Miny% To Maxy%
  If y>0 And y<=239
   Polyx1% = Lefttable((y%), 0)
   Px1 = Lefttable(y%, 1)
   Py1 = Lefttable(y%, 2)

   Polyx2% = Righttable((y%), 0)
   Px2 = Righttable(y%, 1)
   Py2 = Righttable(y%, 2)
   Linewidth% = Polyx2% - Polyx1%
   Linewidth%=Linewidth% Or 1
   Pxadd = ((Px2 - Px1)) / Linewidth%
   Pyadd = ((Py2 - Py1)) / Linewidth%

   For x% = Polyx1% To Polyx2%
     If x>0 And x<=319
      Col%=ReadPixelFast((Px1 Shr 16),(Py1 Shr 16),imgtxt)
      WritePixelFast x%,y%,Col%
     EndIf
     Px1 = Px1 + Pxadd
     Py1 = Py1 + Pyadd
   Next
  EndIf
 Next
End Function

 

.building
Data 23  ; number of pts -1
;    x   y   z

Data -100,-100,-100
Data -100,100,-100
Data -100,100,100
Data -100,-100,100

Data -100,-100,-100
Data  -100,-100,100
Data  100,-100, 100
Data 100,-100, -100

Data -100,100,-100
Data  100,100,-100
Data  100,100, 100
Data -100,100, 100

Data 100,-100,-100
Data 100,-100,100
Data 100,100,100
Data 100,100,-100
 
Data -100,100,100
Data 100,100,100
Data 100,-100,100
Data -100,-100,100

Data -100,100,-100
Data -100,-100,-100
Data 100,-100,-100
Data 100,100,-100