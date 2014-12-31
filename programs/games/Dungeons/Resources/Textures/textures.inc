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

align 4
textures:

.GreenStone1            akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.GreenStone1, 0, 0, 0, AKODE_TEXTURE_USAGE.ENVIRONMENT
.GreenStone2            akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.GreenStone2, 0, 0, 0, AKODE_TEXTURE_USAGE.ENVIRONMENT

.YellowStone1           akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.YellowStone1, 0, 0, 0, AKODE_TEXTURE_USAGE.ENVIRONMENT
.RedStone1              akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.RedStone1, 0, 0, 0, AKODE_TEXTURE_USAGE.ENVIRONMENT

.CrazyWoodDoor          akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.CrazyWoodDoor, 0, 1, 0, AKODE_TEXTURE_USAGE.ENVIRONMENT
.IronDoor               akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.IronDoor, 0, 0, 0, AKODE_TEXTURE_USAGE.ENVIRONMENT

.RedLeverUp             akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.RedLeverUp, 0, 1, 0, AKODE_TEXTURE_USAGE.ENVIRONMENT
.RedLeverDown           akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.RedLeverDown, 0, 1, 0, AKODE_TEXTURE_USAGE.ENVIRONMENT

.PortalFloor            akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.PortalFloor, 0, 0, 0, AKODE_TEXTURE_USAGE.ENVIRONMENT

.Chest                  akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.Chest, 0, 1, 0, AKODE_TEXTURE_USAGE.OBJECT

.Column1                akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.Column1, 0, 1, 0, AKODE_TEXTURE_USAGE.OBJECT
.Column2                akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.Column2, 0, 1, 0, AKODE_TEXTURE_USAGE.OBJECT
.Column3                akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.Column3, 0, 1, 0, AKODE_TEXTURE_USAGE.OBJECT
.Column4                akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.Column4, 0, 1, 0, AKODE_TEXTURE_USAGE.OBJECT

.Ogre                   akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.Ogre, 0, 1, 0, AKODE_TEXTURE_USAGE.OBJECT

.Portal                 akode.TextureDesc       AKODE_TEXTURE_TYPE.IMAGE, texture_files.Portal, 0, 1, 0, AKODE_TEXTURE_USAGE.OBJECT

texture_files:

.GreenStone1            db 'Resources/Textures/Environment/GreenStone1.jpg', 0
.GreenStone2            db 'Resources/Textures/Environment/GreenStone2.jpg', 0

.YellowStone1           db 'Resources/Textures/Environment/YellowStone1.jpg', 0
.RedStone1              db 'Resources/Textures/Environment/RedStone1.jpg', 0

.CrazyWoodDoor          db 'Resources/Textures/Environment/CrazyWoodDoor.png', 0
.IronDoor               db 'Resources/Textures/Environment/IronDoor.jpg', 0

.RedLeverUp             db 'Resources/Textures/Environment/RedLeverUp.png', 0
.RedLeverDown           db 'Resources/Textures/Environment/RedLeverDown.png', 0

.PortalFloor            db 'Resources/Textures/Environment/Portal.jpg', 0

.Chest                  db 'Resources/Textures/Objects/Chest.png', 0

.Column1                db 'Resources/Textures/Objects/Column1.png', 0
.Column2                db 'Resources/Textures/Objects/Column2.png', 0
.Column3                db 'Resources/Textures/Objects/Column3.png', 0
.Column4                db 'Resources/Textures/Objects/Column4.png', 0

.Ogre                   db 'Resources/Textures/Objects/Ogre.png', 0

.Portal                 db 'Resources/Textures/Objects/Portal.png', 0