include 'lang.inc'

struc langstr [lng, data]
{
        if lang eq lng
                sz ., data
        end if
}

struc langstr0 [lng, data]
{
        if lang eq lng
                sz0 ., data
        end if
}

include 'AKODE/data.inc'
include 'Resources/Textures/textures.inc'
include 'Resources/Levels/levels.inc'

FULLSCREEN                      = 0
FSAA                            = 0
DISABLE_SHADING                 = 0

MOVEMENT_SPEED                  = BLOCK_BASE_SIZE + BLOCK_BASE_SIZE / 4

FIELD_OF_VIEW                   = 60
BLOCK_BASE_SIZE                 = 512
BLOCK_HEIGHT                    = 512

INVENTORY_SIZE                  = 10

HUD_PANEL_HEIGHT                = 120

INVENTORY_X                     = 570
INVENTORY_Y                     = 15
INVENTORY_PADDING_X             = 10
INVENTORY_PADDING_Y             = 10
OBJECT_IMAGE_WIDTH              = 40
OBJECT_IMAGE_HEIGHT             = 40

GAME_MESSAGE_X                  = 150 + 15
GAME_MESSAGE_Y                  = 15 + 15
GAME_MESSAGE_COLOR              = 0E4E2ADh

if FULLSCREEN
        ;MAIN_WINDOW_X           = 0
        ;MAIN_WINDOW_Y           = 0
        ;MAIN_WINDOW_WIDTH       = 0FFFFh
        ;MAIN_WINDOW_HEIGHT      = 0FFFFh
        MAIN_WINDOW_STYLE       = (01000001b) shl 24
        MAIN_WINDOW_STYLE2      = 1           shl 24
else
        MAIN_WINDOW_X           = 20
        MAIN_WINDOW_Y           = 20
        MAIN_WINDOW_WIDTH       = 960                   ; client area width
        MAIN_WINDOW_HEIGHT      = 712                   ; client area height
        MAIN_WINDOW_STYLE       = (01110100b) shl 24
        MAIN_WINDOW_STYLE2      = 0
end if

MAIN_EVENT_MASK                 = EVM_REDRAW or EVM_KEY or EVM_BUTTON or EVM_MOUSE or EVM_MOUSE_FILTER

MAIN_WINDOW_TITLE               langstr0            \
                                en, 'Dungeons of Augastes 0.1', \
                                ru, '®¤§¥¬¥«μο €γ£ αβ¥α  0.1'

LevelLoadingImageFile           db 'Resources/Textures/HUD/LevelLoading.png', 0
HudPanelImageFile               db 'Resources/Textures/HUD/Panel.png', 0
DeathImageFile                  db 'Resources/Textures/HUD/Death.png', 0
EndImageFile                    db 'Resources/Textures/HUD/End.png', 0

align 4
LevelLoadingImagePtr            dd 0
HudPanelImagePtr                dd 0
DeathImagePtr                   dd 0
EndImagePtr                     dd 0

ImageBufferPtr                  dd 0

HudPanelNeedsRedraw             dd 0

GameMessage                     dd 0
GameStatus                      dd 0

GAME_STATUS.LEVEL_LOAD_FAILED   = 1
GAME_STATUS.DEAD                = 2
GAME_STATUS.END                 = 3

ACTION.DO_SOMETHING             = AKODE_ACTION.CUSTOM + 1
ACTION.LOOK_AROUND              = AKODE_ACTION.CUSTOM + 2
ACTION.USE_OBJECT               = AKODE_ACTION.CUSTOM + 3
ACTION.LOOK_AT_OBJECT           = AKODE_ACTION.CUSTOM + 4