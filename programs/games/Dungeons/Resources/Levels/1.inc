
levels.level1   akode.LevelHeader       <level1.Width, level1.Height>, <8, 1>, AKODE_DIRECTION.SOUTH, 0FF00FFh, 000000h, BLOCK_BASE_SIZE * 5, 6, 1, level1.init, 0, level1.action

level1:

.Width  = 24
.Height = 25

W equ level1.Wall
w equ level1.OtherWall
_ equ level1.PassableArea
. equ level1.OtherPassableArea
0 equ level1.NotPassableArea
% equ level1.WallWithCrazyWoodDoor
I equ level1.IronDoor
L equ level1.WallWithLever1
q equ level1.WallWithLever2
v equ level1.WallWithLever3
e equ level1.WallWithLever4
c equ level1.ColumnArea
o equ level1.OgreArea
P equ level1.PortalArea

.Grid:
dd W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W
dd W,_,_,W,W,_,_,W,_,W,_,W,_,_,_,_,W,W,W,W,W,W,W,W
dd W,_,_,W,W,_,_,W,_,W,_,W,_,_,_,_,_,_,_,_,_,_,_,W
dd W,_,_,_,_,_,_,W,_,W,_,W,_,_,_,_,_,_,_,_,_,_,_,W
dd W,W,W,_,_,W,W,W,_,W,L,W,_,_,_,W,W,W,_,_,_,W,W,W
dd W,_,_,_,_,_,_,W,_,_,_,W,_,_,_,W,0,W,_,_,_,W,0,W
dd W,_,W,_,W,_,_,W,_,W,W,W,W,W,_,W,c,_,_,_,_,_,c,W
dd W,_,W,_,W,_,_,W,_,W,W,W,W,W,_,W,_,_,_,c,_,_,_,W
dd W,W,W,_,W,W,W,W,%,W,W,W,W,W,_,W,_,_,W,0,W,_,_,W
dd W,_,W,_,W,_,_,_,_,_,_,_,W,W,_,W,_,_,W,W,W,_,_,W
dd W,_,W,W,W,_,_,_,_,_,_,_,I,_,_,W,_,_,_,_,_,_,_,W
dd W,_,_,_,W,_,_,_,_,_,_,_,W,W,W,W,W,W,W,W,W,%,W,W
dd W,_,_,_,W,W,W,W,q,v,e,W,W,_,_,_,_,_,_,_,_,_,_,W
dd W,_,_,_,_,_,_,_,_,_,_,_,W,_,W,W,W,W,W,W,W,W,W,W
dd W,_,_,_,W,_,_,_,_,_,_,_,W,o,W,_,_,_,_,_,_,_,_,W
dd W,_,_,_,W,W,W,W,W,W,W,W,W,0,W,w,w,w,_,_,_,_,_,W
dd W,_,_,_,W,_,_,_,_,_,_,w,_,_,w,w,w,w,w,w,w,w,w,w
dd W,_,_,_,_,_,_,_,_,_,_,w,_,_,w,.,.,.,.,.,.,.,.,w
dd W,_,_,_,_,_,_,_,_,_,_,w,_,_,w,.,.,w,w,.,w,w,.,w
dd W,_,_,_,_,_,_,_,_,_,_,w,_,_,_,.,.,w,.,.,.,w,.,w
dd W,_,_,_,_,_,_,_,_,_,_,w,_,_,_,.,.,.,.,P,.,.,.,w
dd W,_,_,_,_,_,_,_,_,_,_,w,_,_,_,.,.,w,.,.,.,w,.,w
dd W,_,_,_,_,_,_,_,_,_,_,w,_,_,w,.,.,w,w,.,w,w,.,w
dd W,_,_,_,_,_,_,_,_,_,_,w,_,_,w,.,.,.,.,.,.,.,.,w
dd W,W,W,W,W,W,W,W,W,W,W,w,w,w,w,w,w,w,w,w,w,w,w,w

restore W, w, _, ., 0, %, I, L, q, v, e, c, o, P

.Chest                  akode.Object            textures.Chest, <13, 2>, 1, 0
.Column1                akode.Object            textures.Column1, <16, 5>, 1, 0
.Column2                akode.Object            textures.Column2, <19, 8>, 1, 0
.Column3                akode.Object            textures.Column3, <22, 5>, 1, 0
.Ogre                   akode.Object            textures.Ogre, <13, 15>, 1, 0
.Portal                 akode.Object            textures.Portal, <19, 20>, 1, 1

.Column4TextureDescPtr  dd textures.Column4

.Wall                   akode.GridCell          <textures.GreenStone1, 0>, \
                                                <textures.GreenStone1, 0>, \
                                                <textures.GreenStone1, 0>, \
                                                <textures.GreenStone1, 0>, \
                                                <0, 0>,                    \
                                                <0, 0>,                    \
                                                0, 0

.OtherWall              akode.GridCell          <textures.GreenStone2, 0>, \
                                                <textures.GreenStone2, 0>, \
                                                <textures.GreenStone2, 0>, \
                                                <textures.GreenStone2, 0>, \
                                                <0, 0>,                    \
                                                <0, 0>,                    \
                                                0, 0

.WallWithCrazyWoodDoor  akode.GridCell          <textures.GreenStone1, textures.CrazyWoodDoor>, \
                                                <textures.GreenStone1, textures.CrazyWoodDoor>, \
                                                <textures.GreenStone1, textures.CrazyWoodDoor>, \
                                                <textures.GreenStone1, textures.CrazyWoodDoor>, \
                                                <0, 0>,                                         \
                                                <0, 0>,                                         \
                                                0, 0

.IronDoor               akode.GridCell          <textures.IronDoor, 0>, \
                                                <textures.IronDoor, 0>, \
                                                <textures.IronDoor, 0>, \
                                                <textures.IronDoor, 0>, \
                                                <0, 0>,                 \
                                                <0, 0>,                 \
                                                0, 0

.WallWithLever1         akode.GridCell          <textures.GreenStone1, 0>,                      \
                                                <textures.GreenStone1, textures.RedLeverDown>,  \
                                                <textures.GreenStone1, 0>,                      \
                                                <textures.GreenStone1, textures.RedLeverUp>,    \
                                                <0, 0>,                                         \
                                                <0, 0>,                                         \
                                                0, 0

.WallWithLever2         akode.GridCell          <textures.GreenStone1, 0>,                      \
                                                <textures.GreenStone1, textures.RedLeverUp>,    \
                                                <textures.GreenStone1, 0>,                      \
                                                <textures.GreenStone1, textures.RedLeverDown>,  \
                                                <0, 0>,                                         \
                                                <0, 0>,                                         \
                                                0, 0

.WallWithLever3         akode.GridCell          <textures.GreenStone1, 0>,                      \
                                                <textures.GreenStone1, textures.RedLeverUp>,    \
                                                <textures.GreenStone1, 0>,                      \
                                                <textures.GreenStone1, textures.RedLeverDown>,  \
                                                <0, 0>,                                         \
                                                <0, 0>,                                         \
                                                0, 0

.WallWithLever4         akode.GridCell          <textures.GreenStone1, 0>,                      \
                                                <textures.GreenStone1, textures.RedLeverUp>,    \
                                                <textures.GreenStone1, 0>,                      \
                                                <textures.GreenStone1, textures.RedLeverDown>,  \
                                                <0, 0>,                                         \
                                                <0, 0>,                                         \
                                                0, 0

.PassableArea           akode.GridCell          <0, 0>,                     \
                                                <0, 0>,                     \
                                                <0, 0>,                     \
                                                <0, 0>,                     \
                                                <textures.YellowStone1, 0>, \
                                                <textures.YellowStone1, 0>, \
                                                1, 0

.OtherPassableArea      akode.GridCell          <0, 0>,                  \
                                                <0, 0>,                  \
                                                <0, 0>,                  \
                                                <0, 0>,                  \
                                                <textures.RedStone1, 0>, \
                                                <textures.RedStone1, 0>, \
                                                1, 0

.NotPassableArea        akode.GridCell          <0, 0>,                     \
                                                <0, 0>,                     \
                                                <0, 0>,                     \
                                                <0, 0>,                     \
                                                <textures.YellowStone1, 0>, \
                                                <textures.YellowStone1, 0>, \
                                                0, 0

.ColumnArea             akode.GridCell          <0, 0>,                     \
                                                <0, 0>,                     \
                                                <0, 0>,                     \
                                                <0, 0>,                     \
                                                <textures.YellowStone1, 0>, \
                                                <textures.YellowStone1, 0>, \
                                                1, level1.columns

.OgreArea               akode.GridCell          <0, 0>,                     \
                                                <0, 0>,                     \
                                                <0, 0>,                     \
                                                <0, 0>,                     \
                                                <textures.YellowStone1, 0>, \
                                                <textures.YellowStone1, 0>, \
                                                1, level1.ogre

.PortalArea             akode.GridCell          <0, 0>,                    \
                                                <0, 0>,                    \
                                                <0, 0>,                    \
                                                <0, 0>,                    \
                                                <textures.PortalFloor, 0>, \
                                                <textures.RedStone1, 0>,   \
                                                1, level1.portal

.CoinImagePtr           dd 0
.Sun1ImagePtr           dd 0
.Sun2ImagePtr           dd 0
.Sun3ImagePtr           dd 0

proc level1.init
        stdcall load_object_image, .coin_file_path
        mov     [level1.CoinImagePtr], eax
        stdcall load_object_image, .sun1_file_path
        mov     [level1.Sun1ImagePtr], eax
        stdcall load_object_image, .sun2_file_path
        mov     [level1.Sun2ImagePtr], eax
        stdcall load_object_image, .sun3_file_path
        mov     [level1.Sun3ImagePtr], eax

        mov     eax, .message
        ret

.coin_file_path db 'Resources/Textures/Objects/Coin.png', 0
.sun1_file_path db 'Resources/Textures/Objects/Sun1.png', 0
.sun2_file_path db 'Resources/Textures/Objects/Sun2.png', 0
.sun3_file_path db 'Resources/Textures/Objects/Sun3.png', 0

.message                                        langstr0 ru, < \
'Вы - искатель сокровищ. Тяга к наживе привела вас в    ', 10, \
'лесную пещеру, где, по рассказам селян, пару столетий  ', 10, \
'назад, один правитель, спрятал свою казну. В пещере вы ', 10, \
'нашли... дверь. Усилие - дверь открыта! Вы вошли внутрь', 10, \
'и дверь с грохотом захлопнулась. Возможно, это ваш шанс', 10, \
'найти несметные богатства или погибнуть...             '      >

endp

proc level1.action uses ebx ecx edx, action, cell_x, cell_y
        mov     eax, [action]
        mov     ebx, [cell_x]
        mov     ecx, [cell_y]

        mov     edx, ecx
        imul    edx, level1.Width
        add     edx, ebx
        lea     edx, [level1.Grid + edx * 4]

        cmp     ax, ACTION.DO_SOMETHING
        jne     .check_next_action

        cmp     edx, level1.Grid + (7 * level1.Width + 8) * 4
        jne     @f
        cmp     dword [level1.Grid + (8 * level1.Width + 8) * 4], level1.WallWithCrazyWoodDoor
        jne     .exit_no_message

        mov     eax, .message_door1_closed
        jmp     .exit
.message_door1_closed                           langstr0 ru, < \
'Заперто. У двери нет ни ручки, ни замочной скважины.   ', 10, \
'Вероятно, где-то должен быть скрытый механизм...       '      >

@@:     cmp     edx, level1.Grid + (10 * level1.Width + 21) * 4
        jne     @f
        cmp     dword [level1.Grid + (11 * level1.Width + 21) * 4], level1.WallWithCrazyWoodDoor
        jne     .exit_no_message

        mov     eax, .message_door3_closed
        jmp     .exit
.message_door3_closed                           langstr0 ru, < \
'Такую дверь вы уже видели, и в прошлый раз она так     ', 10, \
'просто не открылась.                                   '      >

@@:     cmp     edx, level1.Grid + (5 * level1.Width + 10) * 4
        jne     @f
        cmp     dword [level1.Grid + (8 * level1.Width + 8) * 4], level1.WallWithCrazyWoodDoor
        jne     .exit_no_message

        mov     [level1.WallWithLever1.WallTexture4.DescPtr2], textures.RedLeverDown
        mov     dword [level1.Grid + (8 * level1.Width + 8) * 4], level1.PassableArea

        mov     eax, .message_door1_opens
        jmp     .exit
.message_door1_opens                            langstr0 ru, < \
'Вы слышите звук лебёдки и скрежет шестерёнок.          '      >

@@:     cmp     edx, level1.Grid + (11 * level1.Width + 8) * 4
        jne     @f
        mov     eax, [level1.WallWithLever2.WallTexture2.DescPtr2]
        mov     ebx, [level1.WallWithLever2.WallTexture4.DescPtr2]
        mov     [level1.WallWithLever2.WallTexture2.DescPtr2], ebx
        mov     [level1.WallWithLever2.WallTexture4.DescPtr2], eax
        jmp     .3levers

@@:     cmp     edx, level1.Grid + (11 * level1.Width + 9) * 4
        jne     @f
        mov     eax, [level1.WallWithLever3.WallTexture2.DescPtr2]
        mov     ebx, [level1.WallWithLever3.WallTexture4.DescPtr2]
        mov     [level1.WallWithLever3.WallTexture2.DescPtr2], ebx
        mov     [level1.WallWithLever3.WallTexture4.DescPtr2], eax
        jmp     .3levers

@@:     cmp     edx, level1.Grid + (11 * level1.Width + 10) * 4
        jne     @f
        mov     eax, [level1.WallWithLever4.WallTexture2.DescPtr2]
        mov     ebx, [level1.WallWithLever4.WallTexture4.DescPtr2]
        mov     [level1.WallWithLever4.WallTexture2.DescPtr2], ebx
        mov     [level1.WallWithLever4.WallTexture4.DescPtr2], eax
        jmp     .3levers

@@:     cmp     edx, level1.Grid + (10 * level1.Width + 11) * 4
        jne     @f
        cmp     dword [level1.Grid + (10 * level1.Width + 12) * 4], level1.IronDoor
        jne     .exit_no_message
        mov     eax, .message_door2_closed
        jmp     .exit

@@:     jmp     .exit_no_message

.check_next_action:
        cmp     ax, AKODE_ACTION.CELL_ENTER
        jne     .exit_no_message

        cmp     edx, level1.Grid + (2 * level1.Width + 13) * 4
        jne     @f
        cmp     [level1.Chest.Visible], 1
        jne     .exit_no_message

        stdcall add_object_to_inventory, 4, [level1.CoinImagePtr]
        stdcall add_object_to_inventory, 1, [level1.Sun1ImagePtr]
        stdcall add_object_to_inventory, 2, [level1.Sun2ImagePtr]
        stdcall add_object_to_inventory, 3, [level1.Sun3ImagePtr]

        mov     [level1.Chest.Visible], 0
        mov     eax, .message_chest
        jmp     .exit
.message_chest                                  langstr0 ru, < \
'Вы нашли сундук с барахлом: 1 золотой монеткой и       ', 10, \
'3-мя монетами с изображением солнца в разных фазах.    '      >

@@:     cmp     edx, level1.Grid + (14 * level1.Width + 13) * 4
        jne     @f
        cmp     [level1.Ogre.Visible], 1
        jne     .exit_no_message
        mov     eax, .message_ogre
        jmp     .exit

.message_ogre                                   langstr0 ru, < \
'- Стой! Куда это ты? Дальше хода нет.                  '      >

@@:

.exit_no_message:
        xor     eax, eax

.exit:
        ret

.3levers:
        mov     eax, [level1.WallWithLever2.WallTexture2.DescPtr2]
        mov     ebx, [level1.WallWithLever3.WallTexture2.DescPtr2]
        mov     ecx, [level1.WallWithLever4.WallTexture2.DescPtr2]

        cmp     eax, textures.RedLeverDown
        jne     @f
        cmp     ebx, textures.RedLeverUp
        jne     @f
        cmp     ecx, textures.RedLeverDown
        jne     @f
        mov     dword [level1.Grid + (10 * level1.Width + 12) * 4], level1.PassableArea
        mov     eax, .message_door2_opens
        jmp     .exit

@@:
        mov     eax, .message_3levers
        jmp     .exit

.message_3levers                                langstr0 ru, < \
'Вы переставили этот рычаг в другое положение.          '      >
.message_door2_closed                           langstr0 ru, < \
'Хоть вы и полны сил, но сдвинуть эту дверь хоть на     ', 10, \
'миллиметр у вас не получилось.                         '      >
.message_door2_opens                            langstr0 ru, < \
'До вас донёсся металлический скрежет.                  '      >
endp

proc level1.columns uses ebx ecx, action, cell_x, cell_y
        mov     eax, [action]

        cmp     ax, ACTION.DO_SOMETHING
        jne     @f
        mov     eax, .message_description
        jmp     .exit
.message_description                            langstr0 ru, < \
'Перед вами стоит невысокая колонна с хорошо заметной   ', 10, \
'надписью. Колонна стоит здесь явно не для поддержки    ', 10, \
'потолка...                                             '      >

@@:     cmp     ax, ACTION.USE_OBJECT
        jne     .exit_no_message

        shr     eax, 16
        mov     ebx, eax

        mov     eax, [cell_x]

        cmp     eax, 16
        jne     @f
        cmp     ebx, 1
        jne     .exit_no_message
        stdcall remove_object_from_inventory, 1
        mov     eax, [level1.Column4TextureDescPtr]
        mov     [level1.Column1.TextureDescPtr], eax
        mov     [level1.Column1.DisableShading], 1
        mov     eax, .message_activated
        jmp     .exit

@@:     cmp     eax, 19
        jne     @f
        cmp     ebx, 2
        jne     .exit_no_message
        stdcall remove_object_from_inventory, 2
        mov     eax, [level1.Column4TextureDescPtr]
        mov     [level1.Column2.TextureDescPtr], eax
        mov     [level1.Column2.DisableShading], 1
        mov     eax, .message_activated
        jmp     .exit

@@:     cmp     eax, 22
        jne     @f
        cmp     ebx, 3
        jne     .exit_no_message
        stdcall remove_object_from_inventory, 3
        mov     eax, [level1.Column4TextureDescPtr]
        mov     [level1.Column3.TextureDescPtr], eax
        mov     [level1.Column3.DisableShading], 1
        mov     eax, .message_activated
        jmp     .exit

@@:
.exit_no_message:
        xor     eax, eax
        ret

.exit:
        mov     ecx, [level1.Column1.DisableShading]
        and     ecx, [level1.Column2.DisableShading]
        and     ecx, [level1.Column3.DisableShading]
        jecxz   @f
        mov     dword [level1.Grid + (11 * level1.Width + 21) * 4], level1.PassableArea
@@:
        ret

.message_activated                              langstr0 ru, < \
'Вы положили монету на колонну и она начала светиться   ', 10, \
'странным светом.                                       '      >
endp

proc level1.ogre action, cell_x, cell_y
        mov     eax, [action]

        cmp     ax, ACTION.DO_SOMETHING
        jne     @f
        stdcall player_death
        xor     eax, eax
        jmp     .exit

@@:     cmp     ax, ACTION.USE_OBJECT
        jne     .exit_default
        shr     eax, 16
        cmp     eax, 4
        jne     .exit_default

        mov     dword [level1.Grid + (15 * level1.Width + 13) * 4], level1.PassableArea
        mov     [level1.Ogre.Visible], 0
        stdcall remove_object_from_inventory, 4
        mov     eax, .message_ogre
        jmp     .exit

.exit_default:
        xor     eax, eax
        dec     eax
.exit:
        ret

.message_ogre                                   langstr0 ru, < \
'- Ой! Монетка! А ну вернись!!!                         '      >
endp

proc level1.portal action, cell_x, cell_y
        mov     eax, [action]
        cmp     ax, AKODE_ACTION.CELL_ENTER
        jne     @f
        stdcall game_over
@@:
        xor     eax, eax
        ret
endp