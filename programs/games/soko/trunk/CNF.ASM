include "celltype.inc"

macro Header name, game_type, x, y
{
    db game_type,x,y
  local ..label
    db ..label-$-1
    db name
  ..label:
}

macro C cell,cnt
{
    if cnt eq
        db (cell shl 4)
    else
        db (cell shl 4)+cnt-1
    end if
}


Header 'Sokonex test', sSokonex, 6, 6

; # Wall; + Connector; * Player; B Block

; ######
; #  + #
; #    #
; # ++ #
; #*   #
; ######

C tWall,7

C tEmpty,2
C tConnect
C tEmpty

C tWall,2
C tEmpty,4
C tWall,2
C tEmpty

C tConnect,2
C tEmpty
C tWall,2
C tPlayer

C tEmpty,3
C tWall,7

EndConf

Header 'Just push!', sSokoban, 9, 8 

;   #####  
; ###   ###
; #  @@@  #
; #  # #  #
; ## #B# ##
; # B * B #
; #   #   #
; #########

C tEmpty,2
C tWall,5
C tEmpty,2

C tWall,3
C tEmpty,3
C tWall,4
C tEmpty,2
C tPlace,3
C tEmpty,2
C tWall,2
C tEmpty,2

C tWall
C tEmpty
C tWall
C tEmpty,2
C tWall,3
C tEmpty
C tWall
C tBlock
C tWall
C tEmpty
C tWall,3
C tEmpty
C tBlock
C tEmpty
C tPlayer
C tEmpty
C tBlock
C tEmpty
C tWall,2
C tEmpty,3
C tWall
C tEmpty,3
C tWall,10

EndConf
