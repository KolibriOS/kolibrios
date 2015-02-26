;struct akode.TextureDesc
;        Type            dd ?                            ; AKODE_TEXTURE_TYPE
;
;        union
;                ImagePathPtr    dd ?
;                Color           dd ?
;        ends
;
;        ImageDataPtr    dd ?
;
;        HasMagicPink    db ?                            ; boolean
;        TileWalls       db ?                            ; boolean
;        Usage           db ?                            ; AKODE_TEXTURE_USAGE
;        VirtualScale    db ?                            ; AKODE_VIRTUAL_SCALE_MODE, not supported yet
;        VirtualSize     akode.Size                      ; (0, 0) for original size, not supported yet
;ends

struc TextureImage usage*, filepath*, magic_pink=0
{
        .ImagePath      db 'Resources/Textures/', filepath, 0
        .               akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, .ImagePath, 0, magic_pink, 0, usage
}

struc TextureColor color*
{
        .               akode.TextureDesc       AKODE_TEXTURE_TYPE.COLOR, color, 0, 0, 0, AKODE_TEXTURE_USAGE.ENVIRONMENT
}

align 4
textures:

.GreenStone1            TextureImage            AKODE_TEXTURE_USAGE.ENVIRONMENT, 'Environment/GreenStone1.jpg'
.GreenStone2            TextureImage            AKODE_TEXTURE_USAGE.ENVIRONMENT, 'Environment/GreenStone2.jpg'

.YellowStone1           TextureImage            AKODE_TEXTURE_USAGE.ENVIRONMENT, 'Environment/YellowStone1.jpg'
.RedStone1              TextureImage            AKODE_TEXTURE_USAGE.ENVIRONMENT, 'Environment/RedStone1.jpg'

.CrazyWoodDoor          TextureImage            AKODE_TEXTURE_USAGE.ENVIRONMENT, 'Environment/CrazyWoodDoor.png', 1
.IronDoor               TextureImage            AKODE_TEXTURE_USAGE.ENVIRONMENT, 'Environment/IronDoor.jpg'

.RedLeverUp             TextureImage            AKODE_TEXTURE_USAGE.ENVIRONMENT, 'Environment/RedLeverUp.png', 1
.RedLeverDown           TextureImage            AKODE_TEXTURE_USAGE.ENVIRONMENT, 'Environment/RedLeverDown.png', 1

.PortalFloor            TextureImage            AKODE_TEXTURE_USAGE.ENVIRONMENT, 'Environment/Portal.jpg'

.Chest                  TextureImage            AKODE_TEXTURE_USAGE.OBJECT, 'Objects/Chest.png', 1

.Column1                TextureImage            AKODE_TEXTURE_USAGE.OBJECT, 'Objects/Column1.png', 1
.Column2                TextureImage            AKODE_TEXTURE_USAGE.OBJECT, 'Objects/Column2.png', 1
.Column3                TextureImage            AKODE_TEXTURE_USAGE.OBJECT, 'Objects/Column3.png', 1
.Column4                TextureImage            AKODE_TEXTURE_USAGE.OBJECT, 'Objects/Column4.png', 1

.Ogre                   TextureImage            AKODE_TEXTURE_USAGE.OBJECT, 'Objects/Ogre.png', 1

.Portal                 TextureImage            AKODE_TEXTURE_USAGE.OBJECT, 'Objects/Portal.png', 1