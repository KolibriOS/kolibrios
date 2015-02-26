; ============================ Data definitions ============================== ;

struct akode.Point
        X               dd ?
        Y               dd ?
ends

struct akode.Point3D akode.Point
        Z               dd ?
ends

struct akode.Size
        Width           dd ?
        Height          dd ?
ends

struct akode.Rect
        X               dd ?
        Y               dd ?
        Width           dd ?
        Height          dd ?
ends

struct akode.Camera
        Position        akode.Point3D
        Direction       dd ?
        FieldOfView     dd ?
ends

struct akode.ProjectionPlane
        MidY            dd ?
        Size            akode.Size
ends

struct akode.CombinedTexture                            ; combine 2 textures
        DescPtr1        dd ?                            ; main texture
        DescPtr2        dd ?                            ; 2nd optional texture
ends

AKODE_TEXTURE_TYPE.IMAGE                = 0
AKODE_TEXTURE_TYPE.COLOR                = 1

AKODE_TEXTURE_USAGE.ENVIRONMENT         = 0
AKODE_TEXTURE_USAGE.OBJECT              = 1

AKODE_VIRTUAL_SCALE_MODE.TOPLEFT        = 00h
AKODE_VIRTUAL_SCALE_MODE.TOPCENTER      = 01h
AKODE_VIRTUAL_SCALE_MODE.TOPRIGHT       = 02h
AKODE_VIRTUAL_SCALE_MODE.CENTERLEFT     = 10h
AKODE_VIRTUAL_SCALE_MODE.CENTER         = 11h
AKODE_VIRTUAL_SCALE_MODE.CENTERRIGHT    = 12h
AKODE_VIRTUAL_SCALE_MODE.BOTTOMLEFT     = 20h
AKODE_VIRTUAL_SCALE_MODE.BOTTOMCENTER   = 21h
AKODE_VIRTUAL_SCALE_MODE.BOTTOMRIGHT    = 22h

struct akode.TextureDesc
        Type            dd ?                            ; AKODE_TEXTURE_TYPE

        union
                ImagePathPtr    dd ?
                Color           dd ?
        ends

        ImageDataPtr    dd ?

        HasMagicPink    db ?                            ; boolean
        TileWalls       db ?                            ; boolean, not supported yet
        Usage           db ?                            ; AKODE_TEXTURE_USAGE
        VirtualScale    db ?                            ; AKODE_VIRTUAL_SCALE_MODE, not supported yet
        VirtualSize     akode.Size                      ; (0, 0) for original size, not supported yet
ends

struct akode.GridCell
        WallTexture1    akode.CombinedTexture           ; 4 combined textures for walls
        WallTexture2    akode.CombinedTexture
        WallTexture3    akode.CombinedTexture
        WallTexture4    akode.CombinedTexture

        FloorTexture    akode.CombinedTexture
        CeilingTexture  akode.CombinedTexture

        Passable        dd ?                            ; boolean

        ActionCallback  dd ?                            ; proc callback AKODE_ACTION, cell x, cell y
ends

struct akode.Object
        TextureDescPtr  dd ?
        Position        akode.Point
        Visible         dd ?                            ; boolean
        DisableShading  dd ?                            ; boolean
        Tag             dd ?

        ; for internal use
        Distance        dd ?
        ShadingDistance dd ?
        Angle           dd ?
ends

AKODE_DIRECTION.EAST                    = 0
AKODE_DIRECTION.NORTH                   = 1
AKODE_DIRECTION.WEST                    = 2
AKODE_DIRECTION.SOUTH                   = 3

struct akode.LevelHeader
        Size            akode.Size
        StartPosition   akode.Point
        StartDirection  dd ?                            ; AKODE_DIRECTION
        BackgroundColor dd ?                            ; FF00FF for no background color
        ShadingColor    dd ?
        ShadingDistance dd ?                            ; 0 for no shading

        ObjectCount     dd ?
        TextureCount    dd ?                            ; number of additional textures

        InitCallback    dd ?
        DestroyCallback dd ?
        ActionCallback  dd ?                            ; proc callback AKODE_ACTION, cell x, cell y
ends

AKODE_LEVEL_LOAD.START                  = 0
AKODE_LEVEL_LOAD.END                    = 1
AKODE_LEVEL_LOAD.UNLOADED               = 2

AKODE_ACTION.CELL_LEAVE                 = 0
AKODE_ACTION.CELL_ENTER                 = 1
AKODE_ACTION.CUSTOM                     = 0100h