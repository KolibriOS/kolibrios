format MS COFF
public EXPORTS
section '.flat' code readable align 16

include '../../../../macros.inc'
include '../../../../proc32.inc'



;---------
offs_m_or_i    equ  8 ;смещение параметра 'MM' или 'II' (Motorola, Intel)
offs_id_gr     equ 10 ;смещение id group
offs_id_gr_mak equ 12 ;смещение id group производителя
offs_tag_0     equ  2 ;смещение 0-го тега
tag_size       equ 12 ;размер структуры тега
;форматы данных
tag_format_ui1b  equ  1 ;unsigned integer 1 byte
tag_format_text  equ  2 ;ascii string
tag_format_ui2b  equ  3 ;unsigned integer 2 byte
tag_format_ui4b  equ  4 ;unsigned integer 4 byte
tag_format_urb   equ  5 ;unsigned integer 4/4 byte
tag_format_si1b  equ  6 ;signed integer 1 byte
tag_format_undef equ  7 ;undefined
tag_format_si2b  equ  8 ;signed integer 2 byte
tag_format_si4b  equ  9 ;signed integer 4 byte
tag_format_srb   equ 10 ;signed integer 4/4 byte
tag_format_f4b	 equ 11 ;float 4 byte
tag_format_f8b	 equ 12 ;float 8 byte

align 4
txt_dp db ': ',0
txt_zap db ', ',0
txt_div db '/',0

;заголовок таблиц с группами тегов
align 4
exif_tag_numbers:
dd 0,      gr_0    ;группа app1
dd 0x8769, gr_8769 ;Exif offset
dd 0xa005, gr_a005 ;Interop offset
dd 0x8825, gr_8825 ;GPS info

;групы app2 (здесь у каждого производителя камер свой формат тегов)
dw 0x927c ;app2 для Nikon
db 'Ni'
dd gr_927c_Ni

dw 0x927c ;app2 для Panasonic
db 'Pa'
dd gr_927c_Pa

dw 0x927c ;app2 для Canon
db 'Ca'
dd gr_927c_Ca

dw 0x927c ;app2 для Samsung
db 'Sa'
dd gr_927c_Sa
.end:

align 4
gr_0:
db 0x00,0x0b,'Processing software',0
db 0x00,0xfe,'Subfile type',0
db 0x00,0xff,'OldSubfile type',0
db 0x01,0x00,'Image width',0
db 0x01,0x01,'Image height',0
db 0x01,0x02,'Bits per sample',0
db 0x01,0x03,'Compression',0
db 0x01,0x06,'Photometric interpretation',0
db 0x01,0x07,'Thresholding',0
db 0x01,0x08,'Cell width',0
db 0x01,0x09,'Cell length',0
db 0x01,0x0a,'Fill order',0
db 0x01,0x0d,'Document name',0
db 0x01,0x0e,'Image description',0
db 0x01,0x0f,'Manufacturer of digicam',0
db 0x01,0x10,'Model',0
db 0x01,0x11,'Strip offsets',0
db 0x01,0x12,'Orientation',0
db 0x01,0x15,'Samples per pixel',0
db 0x01,0x16,'Rows per strip',0
db 0x01,0x17,'Strip byte counts',0
db 0x01,0x18,'Min sample value',0
db 0x01,0x19,'Max sample value',0
db 0x01,0x1a,'X resolution',0
db 0x01,0x1b,'Y resolution',0
db 0x01,0x1c,'Planar configuration',0
db 0x01,0x1d,'Page name',0
db 0x01,0x1e,'X position',0
db 0x01,0x1f,'Y position',0
db 0x01,0x20,'Free offsets',0
db 0x01,0x21,'Free byte counts',0
db 0x01,0x22,'Gray response unit',0
db 0x01,0x23,'Gray response curve',0
db 0x01,0x24,'T4 options',0
db 0x01,0x25,'T6 options',0
db 0x01,0x28,'Resolution unit',0
db 0x01,0x29,'Page number',0
db 0x01,0x2c,'Color response unit',0
db 0x01,0x2d,'Transfer function',0
db 0x01,0x31,'Software',0
db 0x01,0x32,'Modify date',0
db 0x01,0x3b,'Artist',0
db 0x01,0x3c,'Host computer',0
db 0x01,0x3d,'Predictor',0
db 0x01,0x3e,'White point',0
db 0x01,0x3f,'Primary chromaticities',0
db 0x01,0x40,'Color map',0
db 0x01,0x41,'Halftone hints',0
db 0x01,0x42,'Tile width',0
db 0x01,0x43,'Tile length',0
db 0x01,0x44,'Tile offsets',0
db 0x01,0x45,'Tile byte counts',0
db 0x01,0x46,'Bad fax lines',0
db 0x01,0x47,'Clean fax data',0
db 0x01,0x48,'Consecutive bad fax lines',0
db 0x01,0x4a,'Sub IFDs',0
db 0x01,0x4c,'Ink set',0
db 0x01,0x4d,'Ink names',0
db 0x01,0x4e,'Numberof inks',0
db 0x01,0x50,'Dot range',0
db 0x01,0x51,'Target printer',0
db 0x01,0x52,'Extra samples',0
db 0x01,0x53,'Sample format',0
db 0x01,0x54,'SMin sample value',0
db 0x01,0x55,'SMax sample value',0
db 0x01,0x56,'Transfer range',0
db 0x01,0x57,'Clip path',0
db 0x01,0x58,'X clip path units',0
db 0x01,0x59,'Y clip path units',0
db 0x01,0x5a,'Indexed',0
db 0x01,0x5b,'JPEG tables',0
db 0x01,0x5f,'OPIProxy',0
db 0x01,0x90,'Global parameters IFD',0
db 0x01,0x91,'Profile type',0
db 0x01,0x92,'Fax profile',0
db 0x01,0x93,'Coding methods',0
db 0x01,0x94,'Version year',0
db 0x01,0x95,'Mode number',0
db 0x01,0xb1,'Decode',0
db 0x01,0xb2,'Default image color',0
db 0x01,0xb3,'T82 options',0
db 0x01,0xb5,'JPEG tables',0 ;уже было ?
db 0x02,0x00,'JPEG proc',0
db 0x02,0x01,'Thumbnail offset',0
db 0x02,0x02,'Thumbnail length',0
db 0x02,0x03,'JPEG restart interval',0
db 0x02,0x05,'JPEG lossless predictors',0
db 0x02,0x06,'JPEG point transforms',0
db 0x02,0x07,'JPEG QTables',0
db 0x02,0x08,'JPEG DCTables',0
db 0x02,0x09,'JPEG ACTables',0
db 0x02,0x11,'YCbCrCoefficients',0
db 0x02,0x12,'YCbCrSubSampling',0
db 0x02,0x13,'YCbCrPositioning',0
db 0x02,0x14,'Reference black white',0
db 0x02,0x2f,'Strip row counts',0
db 0x03,0xe7,'USPTO Miscellaneous',0
db 0x47,0x46,'Rating',0
db 0x47,0x47,'XP_DIP_XML',0
db 0x47,0x48,'Stitch info',0
db 0x47,0x49,'Rating percent',0
db 0x80,0x0d,'Image ID',0
db 0x80,0xa3,'Wang tag 1',0
db 0x80,0xa4,'Wang annotation',0
db 0x80,0xa5,'Wang tag 3',0
db 0x80,0xa6,'Wang tag 4',0
db 0x80,0xe3,'Matteing',0
db 0x80,0xe4,'Data type',0
db 0x80,0xe5,'Image depth',0
db 0x80,0xe6,'Tile depth',0
db 0x82,0x7d,'Model 2',0
db 0x82,0x8d,'CFA repeat pattern dim',0
db 0x82,0x8e,'CFA pattern 2',0
db 0x82,0x8f,'Battery level',0
db 0x82,0x90,'Kodak IFD',0
db 0x82,0x98,'Copyright',0
db 0x82,0xa5,'MD file tag',0
db 0x82,0xa6,'MD scale pixel',0
db 0x82,0xa7,'MD color table',0
db 0x82,0xa8,'MD lab name',0
db 0x82,0xa9,'MD sample info',0
db 0x82,0xaa,'MD prep date',0
db 0x82,0xab,'MD prep time',0
db 0x82,0xac,'MD file units',0
db 0x83,0x0e,'Pixel scale',0
db 0x83,0x35,'Advent scale',0
db 0x83,0x36,'Advent revision',0
db 0x83,0x5c,'UIC1 tag',0
db 0x83,0x5d,'UIC2 tag',0
db 0x83,0x5e,'UIC3 tag',0
db 0x83,0x5f,'UIC4 tag',0
db 0x83,0xbb,'IPTC-NAA',0
db 0x84,0x7e,'Intergraph packet data',0
db 0x84,0x7f,'Intergraph flag registers',0
db 0x84,0x80,'Intergraph matrix',0
db 0x84,0x81,'INGR reserved',0
db 0x84,0x82,'Model tie point',0
db 0x84,0xe0,'Site',0
db 0x84,0xe1,'Color sequence',0
db 0x84,0xe2,'IT8 header',0
db 0x84,0xe3,'Raster padding',0
db 0x84,0xe4,'Bits per run length',0
db 0x84,0xe5,'Bits per extended run length',0
db 0x84,0xe6,'Color table',0
db 0x84,0xe7,'Image color indicator',0
db 0x84,0xe8,'Background color indicator',0
db 0x84,0xe9,'Image color value',0
db 0x84,0xea,'Background color value',0
db 0x84,0xeb,'Pixel intensity range',0
db 0x84,0xec,'Transparency indicator',0
db 0x84,0xed,'Color characterization',0
db 0x84,0xee,'HCUsage',0
db 0x84,0xef,'Trap indicator',0
db 0x84,0xf0,'CMYK equivalent',0
db 0x85,0x46,'SEM info',0
db 0x85,0x68,'AFCP_IPTC',0
db 0x85,0xb8,'Pixel magic JBIG options',0
db 0x85,0xd8,'Model transform',0
db 0x86,0x02,'WB_GRGB levels',0
db 0x86,0x06,'Leaf data',0
db 0x86,0x49,'Photoshop settings',0
db 0x87,0x69,'Exif offset',0
db 0x87,0x73,'ICC_Profile',0
db 0x87,0x7f,'TIFF_FX extensions',0
db 0x87,0x80,'Multi profiles',0
db 0x87,0x81,'Shared data',0
db 0x87,0x82,'T88 options',0
db 0x87,0xac,'Image layer',0
db 0x87,0xaf,'Geo tiff directory',0
db 0x87,0xb0,'Geo tiff double params',0
db 0x87,0xb1,'Geo tiff ascii params',0
db 0x88,0x25,'GPS info',0
db 0x88,0x28,'Opto-Electric conv factor',0
db 0x88,0x29,'Interlace',0
db 0x88,0x5c,'Fax recv params',0
db 0x88,0x5d,'Fax sub address',0
db 0x88,0x5e,'Fax recv time',0
db 0x88,0x8a,'Leaf sub IFD',0
db 0x92,0x0b,'Flash energy',0
db 0x92,0x0c,'Spatial frequency response',0
db 0x92,0x0d,'Noise',0
db 0x92,0x0e,'Focal plane X resolution',0
db 0x92,0x0f,'Focal plane Y resolution',0
db 0x92,0x10,'Focal plane resolution unit',0
db 0x92,0x15,'Exposure index',0
db 0x92,0x16,'TIFF-EP standard ID',0
db 0x92,0x17,'Sensing method',0
db 0x92,0x3a,'CIP3 data file',0
db 0x92,0x3b,'CIP3 sheet',0
db 0x92,0x3c,'CIP3 side',0
db 0x92,0x3f,'Sto nits',0
db 0x93,0x2f,'MS document text',0
db 0x93,0x30,'MS property set storage',0
db 0x93,0x31,'MS document text position',0
db 0x93,0x5c,'Image source data',0
db 0x9c,0x9b,'XP title',0
db 0x9c,0x9c,'XP comment',0
db 0x9c,0x9d,'XP author',0
db 0x9c,0x9e,'XP keywords',0
db 0x9c,0x9f,'XP subject',0
db 0xa2,0x0c,'Spatial frequency fesponse',0
db 0xa2,0x0d,'Noise',0
db 0xa2,0x11,'Image number',0
db 0xa2,0x12,'Security classification',0
db 0xa2,0x13,'Image history',0
db 0xa2,0x16,'TIFF-EP standard ID',0
db 0xa4,0x80,'GDAL metadata',0
db 0xa4,0x81,'GDAL no data',0
db 0xaf,0xc0,'Expand software',0
db 0xaf,0xc1,'Expand lens',0
db 0xaf,0xc2,'Expand film',0
db 0xaf,0xc3,'Expand filterLens',0
db 0xaf,0xc4,'Expand scanner',0
db 0xaf,0xc5,'Expand flash lamp',0
db 0xbc,0x01,'Pixel format',0
db 0xbc,0x02,'Transformation',0
db 0xbc,0x03,'Uncompressed',0
db 0xbc,0x04,'Image type',0
db 0xbc,0x80,'Image width',0
db 0xbc,0x81,'Image height',0
db 0xbc,0x82,'Width resolution',0
db 0xbc,0x83,'Height resolution',0
db 0xbc,0xc0,'Image offset',0
db 0xbc,0xc1,'Image byte count',0
db 0xbc,0xc2,'Alpha offset',0
db 0xbc,0xc3,'Alpha byte count',0
db 0xbc,0xc4,'Image data discard',0
db 0xbc,0xc5,'Alpha data discard',0
db 0xc4,0x27,'Oce scanjob desc',0
db 0xc4,0x28,'Oce application selector',0
db 0xc4,0x29,'Oce ID number',0
db 0xc4,0x2a,'Oce image logic',0
db 0xc4,0x4f,'Annotations',0
db 0xc4,0xa5,'Print IM',0
db 0xc5,0x73,'Original file name',0
db 0xc5,0x80,'USPTO original content type',0
db 0xc6,0x12,'DNG version',0
db 0xc6,0x13,'DNG backward version',0
db 0xc6,0x14,'Unique camera model',0
db 0xc6,0x15,'Localized camera model',0
db 0xc6,0x16,'CFA plane color',0
db 0xc6,0x17,'CFA layout',0
db 0xc6,0x18,'Linearization table',0
db 0xc6,0x19,'Black level repeat dim',0
db 0xc6,0x1a,'Black level',0
db 0xc6,0x1b,'Black level delta H',0
db 0xc6,0x1c,'Black level delta V',0
db 0xc6,0x1d,'White level',0
db 0xc6,0x1e,'Default scale',0
db 0xc6,0x1f,'Default crop origin',0
db 0xc6,0x20,'Default crop size',0
db 0xc6,0x21,'Color matrix 1',0
db 0xc6,0x22,'Color matrix 2',0
db 0xc6,0x23,'Camera calibration 1',0
db 0xc6,0x24,'Camera calibration 2',0
db 0xc6,0x25,'Reduction matrix 1',0
db 0xc6,0x26,'Reduction matrix 2',0
db 0xc6,0x27,'Analog balance',0
db 0xc6,0x28,'As shot neutral',0
db 0xc6,0x29,'As shot white XY',0
db 0xc6,0x2a,'BaselineExposure',0
db 0xc6,0x2b,'BaselineNoise',0
db 0xc6,0x2c,'BaselineSharpness',0
db 0xc6,0x2d,'BayerGreenSplit',0
db 0xc6,0x2e,'Linear response limit',0
db 0xc6,0x2f,'Camera serial number',0
db 0xc6,0x30,'DNG lens info',0
db 0xc6,0x31,'Chroma blur radius',0
db 0xc6,0x32,'Anti alias strength',0
db 0xc6,0x33,'Shadow scale',0
db 0xc6,0x34,'SR2 private',0
db 0xc6,0x35,'Maker note safety',0
db 0xc6,0x40,'Raw image segmentation',0
db 0xc6,0x5a,'Calibration illuminant 1',0
db 0xc6,0x5b,'Calibration illuminant 2',0
db 0xc6,0x5c,'Best quality scale',0
db 0xc6,0x5d,'Raw data unique ID',0
db 0xc6,0x60,'Alias layer metadata',0
db 0xc6,0x8b,'Original raw file name',0
db 0xc6,0x8c,'Original raw file data',0
db 0xc6,0x8d,'Active area',0
db 0xc6,0x8e,'Masked areas',0
db 0xc6,0x8f,'AsShot ICC profile',0
db 0xc6,0x90,'AsShot pre profile matrix',0
db 0xc6,0x91,'Current ICC profile',0
db 0xc6,0x92,'Current pre profile matrix',0
db 0xc6,0xbf,'Colorimetric reference',0
db 0xc6,0xd2,'Panasonic title',0
db 0xc6,0xd3,'Panasonic title 2',0
db 0xc6,0xf3,'Camera calibration sig',0
db 0xc6,0xf4,'Profile calibration sig',0
db 0xc6,0xf5,'Profile IFD',0
db 0xc6,0xf6,'AsShot profile name',0
db 0xc6,0xf7,'Noise reduction applied',0
db 0xc6,0xf8,'Profile name',0
db 0xc6,0xf9,'Profile hue sat map dims',0
db 0xc6,0xfa,'Profile hue sat map data 1',0
db 0xc6,0xfb,'Profile hue sat map data 2',0
db 0xc6,0xfc,'Profile tone curve',0
db 0xc6,0xfd,'Profile embed policy',0
db 0xc6,0xfe,'Profile copyright',0
db 0xc7,0x14,'Forward matrix 1',0
db 0xc7,0x15,'Forward matrix 2',0
db 0xc7,0x16,'Preview application name',0
db 0xc7,0x17,'Preview application version',0
db 0xc7,0x18,'Preview settings name',0
db 0xc7,0x19,'Preview settings digest',0
db 0xc7,0x1a,'Preview color space',0
db 0xc7,0x1b,'Preview date time',0
db 0xc7,0x1c,'Raw image digest',0
db 0xc7,0x1d,'Original raw file digest',0
db 0xc7,0x1e,'Sub tile block size',0
db 0xc7,0x1f,'Row interleave factor',0
db 0xc7,0x25,'Profile look table dims',0
db 0xc7,0x26,'Profile look table data',0
db 0xc7,0x40,'Opcode list 1',0
db 0xc7,0x41,'Opcode list 2',0
db 0xc7,0x4e,'Opcode list 3',0
db 0xc7,0x61,'Noise profile',0
db 0xc7,0x63,'Time codes',0
db 0xc7,0x64,'Frame rate',0
db 0xc7,0x72,'TStop',0
db 0xc7,0x89,'Reel name',0
db 0xc7,0x91,'Original default final size',0
db 0xc7,0x92,'Original best quality size',0
db 0xc7,0x93,'Original default crop size',0
db 0xc7,0xa1,'Camera label',0
db 0xc7,0xa3,'Profile hue sat map encoding',0
db 0xc7,0xa4,'Profile look table encoding',0
db 0xc7,0xa5,'Baseline exposure offset',0
db 0xc7,0xa6,'Default black render',0
db 0xc7,0xa7,'New raw image digest',0
db 0xc7,0xa8,'Raw to preview gain',0
db 0xc7,0xb5,'Default user crop',0
db 0xfe,0x00,'KDC_IFD',0

dd 0

align 4
gr_8769:
db 0x02,0xbc,'Application notes',0
db 0x82,0x9a,'Exposure time',0
db 0x82,0x9d,'F number',0
db 0x88,0x22,'Exposure program',0
db 0x88,0x24,'Spectral sensitivity',0
db 0x88,0x27,'ISO',0
db 0x88,0x2a,'Time zone offset',0
db 0x88,0x2b,'Self timer mode',0
db 0x88,0x30,'Sensitivity type',0
db 0x88,0x31,'Standard output sensitivity',0
db 0x88,0x32,'Recommended exposure index',0
db 0x88,0x33,'ISO speed',0
db 0x88,0x34,'ISO speed latitude yyy',0
db 0x88,0x35,'ISO speed latitude zzz',0
db 0x90,0x00,'Exif version',0
db 0x90,0x03,'Date time original',0
db 0x90,0x04,'Create date',0
db 0x91,0x01,'Components configuration',0
db 0x91,0x02,'Compressed bits per pixel',0
db 0x92,0x01,'Shutter speed value',0
db 0x92,0x02,'Aperture value',0
db 0x92,0x03,'Brightness value',0
db 0x92,0x04,'Exposure compensation',0
db 0x92,0x05,'Max aperture value',0
db 0x92,0x06,'Subject distance',0
db 0x92,0x07,'Metering mode',0
db 0x92,0x08,'Light source',0
db 0x92,0x09,'Flash',0
db 0x92,0x0a,'Focal length',0
db 0x92,0x11,'Image number',0
db 0x92,0x12,'Security classification',0
db 0x92,0x13,'Image history',0
db 0x92,0x14,'Subject area',0
db 0x92,0x7c,'Maker note',0
db 0x92,0x86,'User comment',0
db 0x92,0x90,'Sub sec time',0
db 0x92,0x91,'Sub sec time original',0
db 0x92,0x92,'Sub sec time digitized',0
db 0xa0,0x00,'Flashpix version',0
db 0xa0,0x01,'Color space',0
db 0xa0,0x02,'Exif image width',0
db 0xa0,0x03,'Exif image height',0
db 0xa0,0x04,'Related sound file',0
db 0xa0,0x05,'Interop offset',0
db 0xa2,0x0b,'Flash energy',0
db 0xa2,0x0e,'Focal plane X resolution',0
db 0xa2,0x0f,'Focal plane Y resolution',0
db 0xa2,0x10,'Focal plane resolution unit',0
db 0xa2,0x14,'Subject location',0
db 0xa2,0x15,'Exposure index',0
db 0xa2,0x17,'Sensing method',0
db 0xa3,0x00,'File source',0
db 0xa3,0x01,'Scene type',0
db 0xa3,0x02,'CFA pattern',0
db 0xa4,0x01,'Custom rendered',0
db 0xa4,0x02,'Exposure mode',0
db 0xa4,0x03,'White balance',0
db 0xa4,0x04,'Digital zoom ratio',0
db 0xa4,0x05,'Focal length in 35mm format',0
db 0xa4,0x06,'Scene capture type',0
db 0xa4,0x07,'Gain control',0
db 0xa4,0x08,'Contrast',0
db 0xa4,0x09,'Saturation',0
db 0xa4,0x0a,'Sharpness',0
db 0xa4,0x0b,'Device setting description',0
db 0xa4,0x0c,'Subject distance range',0
db 0xa4,0x20,'Image unique ID',0
db 0xa4,0x30,'Owner name',0
db 0xa4,0x31,'Serial number',0
db 0xa4,0x32,'Lens info',0
db 0xa4,0x33,'Lens make',0
db 0xa4,0x34,'Lens model',0
db 0xa4,0x35,'Lens serial number',0
db 0xa5,0x00,'Gamma',0
db 0xea,0x1c,'Padding',0
db 0xea,0x1d,'Offset schema',0
db 0xfd,0xe8,'Owner name',0
db 0xfd,0xe9,'Serial number',0
db 0xfd,0xea,'Lens',0
db 0xfe,0x4c,'Raw file',0
db 0xfe,0x4d,'Converter',0
db 0xfe,0x4e,'White balance',0
db 0xfe,0x51,'Exposure',0
db 0xfe,0x52,'Shadows',0
db 0xfe,0x53,'Brightness',0
db 0xfe,0x54,'Contrast',0
db 0xfe,0x55,'Saturation',0
db 0xfe,0x56,'Sharpness',0
db 0xfe,0x57,'Smoothness',0
db 0xfe,0x58,'Moire filter',0

dd 0

align 4
gr_a005:
db 0x00,0x01,'Interop index',0
db 0x00,0x02,'Interop version',0
db 0x10,0x00,'Related image file format',0
db 0x10,0x01,'Related image width',0
db 0x10,0x02,'Related image height',0
dd 0

align 4
gr_8825:
db 0x00,0x00,'GPS version ID',0
db 0x00,0x01,'GPS latitude ref',0
db 0x00,0x02,'GPS latitude',0
db 0x00,0x03,'GPS longitude ref',0
db 0x00,0x04,'GPS longitude',0
db 0x00,0x05,'GPS altitude ref',0
db 0x00,0x06,'GPS altitude',0
db 0x00,0x07,'GPS time stamp',0
db 0x00,0x08,'GPS satellites',0
db 0x00,0x09,'GPS status',0
db 0x00,0x0a,'GPS measuremode',0
db 0x00,0x0b,'GPS dop',0
db 0x00,0x0c,'GPS speed ref',0
db 0x00,0x0d,'GPS speed',0
db 0x00,0x0e,'GPS track ref',0
db 0x00,0x0f,'GPS track',0
db 0x00,0x10,'GPS img direction ref',0
db 0x00,0x11,'GPS img direction',0
db 0x00,0x12,'GPS map datum',0
db 0x00,0x13,'GPS dest latitude ref',0
db 0x00,0x14,'GPS dest latitude',0
db 0x00,0x15,'GPS dest longitude ref',0
db 0x00,0x16,'GPS dest longitude',0
db 0x00,0x17,'GPS dest bearing ref',0
db 0x00,0x18,'GPS dest bearing',0
db 0x00,0x19,'GPS dest distance ref',0
db 0x00,0x1a,'GPS dest distance',0
db 0x00,0x1b,'GPS processing method',0
db 0x00,0x1c,'GPS area information',0
db 0x00,0x1d,'GPS date stamp',0
db 0x00,0x1e,'GPS differential',0
db 0x00,0x1f,'GPS h positioning error',0

dd 0

;данные app2 для Nikon
align 4
gr_927c_Ni:
db 0x00,0x01,'MakerNoteVersion',0
db 0x00,0x02,'ISO',0
db 0x00,0x03,'ColorMode',0
db 0x00,0x04,'Quality',0
db 0x00,0x05,'WhiteBalance',0
db 0x00,0x06,'Sharpness',0
db 0x00,0x07,'FocusMode',0
db 0x00,0x08,'FlashSetting',0
db 0x00,0x09,'FlashType',0
db 0x00,0x0b,'WhiteBalanceFineTune',0
db 0x00,0x0c,'WB_RBLevels',0
db 0x00,0x0d,'ProgramShift',0
db 0x00,0x0e,'ExposureDifference',0
db 0x00,0x0f,'ISOSelection',0
db 0x00,0x10,'DataDump',0
db 0x00,0x11,'PreviewIFD',0
db 0x00,0x12,'FlashExposureComp',0
db 0x00,0x13,'ISOSetting',0
db 0x00,0x14,'ColorBalanceA ',0
db 0x00,0x16,'ImageBoundary',0
db 0x00,0x17,'ExternalFlashExposureComp',0
db 0x00,0x18,'FlashExposureBracketValue',0
db 0x00,0x19,'ExposureBracketValue',0
db 0x00,0x1a,'ImageProcessing',0
db 0x00,0x1b,'CropHiSpeed',0
db 0x00,0x1c,'ExposureTuning',0
db 0x00,0x1d,'SerialNumber',0
db 0x00,0x1e,'ColorSpace',0
db 0x00,0x1f,'VRInfo',0
db 0x00,0x20,'ImageAuthentication',0
db 0x00,0x21,'FaceDetect',0
db 0x00,0x22,'ActiveD-Lighting',0
db 0x00,0x23,'PictureControlData',0
db 0x00,0x24,'WorldTime',0
db 0x00,0x25,'ISOInfo',0
db 0x00,0x2a,'VignetteControl',0
db 0x00,0x2b,'DistortInfo',0
db 0x00,0x2c,'UnknownInfo',0
db 0x00,0x32,'UnknownInfo2',0
db 0x00,0x35,'HDRInfo',0
db 0x00,0x39,'LocationInfo',0
db 0x00,0x3d,'BlackLevel',0
db 0x00,0x80,'ImageAdjustment',0
db 0x00,0x81,'ToneComp',0
db 0x00,0x82,'AuxiliaryLens',0
db 0x00,0x83,'LensType',0
db 0x00,0x84,'Lens',0
db 0x00,0x85,'ManualFocusDistance',0
db 0x00,0x86,'DigitalZoom',0
db 0x00,0x87,'FlashMode',0
db 0x00,0x88,'AFInfo',0
db 0x00,0x89,'ShootingMode',0
db 0x00,0x8b,'LensFStops',0
db 0x00,0x8c,'ContrastCurve',0
db 0x00,0x8d,'ColorHue',0
db 0x00,0x8f,'SceneMode',0
db 0x00,0x90,'LightSource',0
db 0x00,0x91,'ShotInfo',0
db 0x00,0x92,'HueAdjustment',0
db 0x00,0x93,'NEFCompression',0
db 0x00,0x94,'Saturation',0
db 0x00,0x95,'NoiseReduction',0
db 0x00,0x96,'NEFLinearizationTable',0
db 0x00,0x97,'ColorBalance',0
db 0x00,0x98,'LensData',0
db 0x00,0x99,'RawImageCenter',0
db 0x00,0x9a,'SensorPixelSize',0
db 0x00,0x9c,'SceneAssist',0
db 0x00,0x9e,'RetouchHistory',0
db 0x00,0xa0,'SerialNumber',0
db 0x00,0xa2,'ImageDataSize',0
db 0x00,0xa5,'ImageCount',0
db 0x00,0xa6,'DeletedImageCount',0
db 0x00,0xa7,'ShutterCount',0
db 0x00,0xa8,'FlashInfo',0
db 0x00,0xa9,'ImageOptimization',0
db 0x00,0xaa,'Saturation',0
db 0x00,0xab,'VariProgram',0
db 0x00,0xac,'ImageStabilization',0
db 0x00,0xad,'AFResponse',0
db 0x00,0xb0,'MultiExposure',0
db 0x00,0xb1,'HighISONoiseReduction',0
db 0x00,0xb3,'ToningEffect',0
db 0x00,0xb6,'PowerUpTime',0
db 0x00,0xb7,'AFInfo2',0
db 0x00,0xb8,'FileInfo',0
db 0x00,0xb9,'AFTune',0
db 0x00,0xbd,'PictureControlData',0
db 0x00,0xc3,'BarometerInfo',0
db 0x0e,0x00,'PrintIM',0
db 0x0e,0x01,'NikonCaptureData',0
db 0x0e,0x09,'NikonCaptureVersion',0
db 0x0e,0x0e,'NikonCaptureOffsets',0
db 0x0e,0x10,'NikonScanIFD',0
db 0x0e,0x13,'NikonCaptureEditVersions ',0
db 0x0e,0x1d,'NikonICCProfile',0
db 0x0e,0x1e,'NikonCaptureOutput',0
db 0x0e,0x22,'NEFBitDepth',0

dd 0

;данные app2 для Panasonic
align 4
gr_927c_Pa:
db 0x00,0x01,'ImageQuality',0
db 0x00,0x02,'FirmwareVersion',0
db 0x00,0x03,'WhiteBalance',0
db 0x00,0x07,'FocusMode',0
db 0x00,0x0f,'AFAreaMode',0
db 0x00,0x1a,'ImageStabilization',0
db 0x00,0x1c,'MacroMode',0
db 0x00,0x1f,'ShootingMode',0
db 0x00,0x20,'Audio',0
db 0x00,0x21,'DataDump',0
db 0x00,0x23,'WhiteBalanceBias',0
db 0x00,0x24,'FlashBias',0
db 0x00,0x25,'InternalSerialNumber',0
db 0x00,0x26,'PanasonicExifVersion',0
db 0x00,0x28,'ColorEffect',0
db 0x00,0x29,'TimeSincePowerOn',0
db 0x00,0x2a,'BurstMode',0
db 0x00,0x2b,'SequenceNumber',0
db 0x00,0x2c,'ContrastMode',0
db 0x00,0x2d,'NoiseReduction',0
db 0x00,0x2e,'SelfTimer',0
db 0x00,0x30,'Rotation',0
db 0x00,0x31,'AFAssistLamp',0
db 0x00,0x32,'ColorMode',0
db 0x00,0x33,'BabyAge',0
db 0x00,0x34,'OpticalZoomMode',0
db 0x00,0x35,'ConversionLens',0
db 0x00,0x36,'TravelDay',0
db 0x00,0x39,'Contrast',0
db 0x00,0x3a,'WorldTimeLocation',0
db 0x00,0x3b,'TextStamp',0
db 0x00,0x3c,'ProgramISO',0
db 0x00,0x3d,'AdvancedSceneType',0
db 0x00,0x3f,'FacesDetected',0
db 0x00,0x40,'Saturation',0
db 0x00,0x41,'Sharpness',0
db 0x00,0x42,'FilmMode',0
db 0x00,0x44,'ColorTempKelvin',0
db 0x00,0x45,'BracketSettings',0
db 0x00,0x46,'WBShiftAB',0
db 0x00,0x47,'WBShiftGM',0
db 0x00,0x48,'FlashCurtain',0
db 0x00,0x49,'LongExposureNoiseReduction',0
db 0x00,0x4b,'PanasonicImageWidth',0
db 0x00,0x4c,'PanasonicImageHeight',0
db 0x00,0x4d,'AFPointPosition',0
db 0x00,0x51,'LensType',0
db 0x00,0x52,'LensSerialNumber',0
db 0x00,0x53,'AccessoryType',0
db 0x00,0x54,'AccessorySerialNumber',0
db 0x00,0x59,'Transform',0
db 0x00,0x5d,'IntelligentExposure',0
db 0x00,0x60,'LensFirmwareVersion',0
db 0x00,0x61,'FaceRecInfo',0
db 0x00,0x62,'FlashWarning',0
db 0x00,0x63,'RecognizedFaceFlags',0
db 0x00,0x65,'Title',0
db 0x00,0x66,'BabyName',0
db 0x00,0x67,'Location',0
db 0x00,0x69,'Country',0
db 0x00,0x6b,'State',0
db 0x00,0x6d,'City',0
db 0x00,0x6f,'Landmark',0
db 0x00,0x70,'IntelligentResolution',0
db 0x00,0x77,'BurstSpeed',0
db 0x00,0x79,'IntelligentD-Range',0
db 0x00,0x7c,'ClearRetouch',0
db 0x00,0x86,'ManometerPressure',0
db 0x00,0x89,'PhotoStyle',0
db 0x00,0x8a,'ShadingCompensation',0
db 0x00,0x8c,'AccelerometerZ',0
db 0x00,0x8d,'AccelerometerX',0
db 0x00,0x8e,'AccelerometerY',0
db 0x00,0x8f,'CameraOrientation',0
db 0x00,0x90,'RollAngle',0
db 0x00,0x91,'PitchAngle',0
db 0x00,0x93,'SweepPanoramaDirection',0
db 0x00,0x94,'SweepPanoramaFieldOfView',0
db 0x00,0x96,'TimerRecording',0
db 0x00,0x9d,'InternalNDFilter',0
db 0x00,0x9e,'HDR',0
db 0x00,0x9f,'ShutterType',0
db 0x00,0xa3,'ClearRetouchValue',0
db 0x00,0xab,'TouchAE',0
db 0x0e,0x00,'PrintIM',0
db 0x80,0x00,'MakerNoteVersion',0
db 0x80,0x01,'SceneMode',0
db 0x80,0x04,'WBRedLevel',0
db 0x80,0x05,'WBGreenLevel',0
db 0x80,0x06,'WBBlueLevel',0
db 0x80,0x07,'FlashFired',0
db 0x80,0x08,'TextStamp',0
db 0x80,0x09,'TextStamp',0
db 0x80,0x10,'BabyAge',0
db 0x80,0x12,'Transform',0

dd 0

;данные app2 для Canon
align 4
gr_927c_Ca:
db 0x00,0x01,'CanonCameraSettings',0
db 0x00,0x02,'CanonFocalLength',0
db 0x00,0x03,'CanonFlashInfo?',0
db 0x00,0x04,'CanonShotInfo',0
db 0x00,0x05,'CanonPanorama',0
db 0x00,0x06,'CanonImageType',0
db 0x00,0x07,'CanonFirmwareVersion',0
db 0x00,0x08,'FileNumber',0
db 0x00,0x09,'OwnerName',0
db 0x00,0x0a,'UnknownD30',0
db 0x00,0x0c,'SerialNumber',0
db 0x00,0x0d,'CanonCameraInfo',0
db 0x00,0x0e,'CanonFileLength',0
db 0x00,0x0f,'CustomFunctions',0
db 0x00,0x10,'CanonModelID',0
db 0x00,0x11,'MovieInfo',0
db 0x00,0x12,'CanonAFInfo',0
db 0x00,0x13,'ThumbnailImageValidArea',0
db 0x00,0x15,'SerialNumberFormat',0
db 0x00,0x1a,'SuperMacro',0
db 0x00,0x1c,'DateStampMode',0
db 0x00,0x1d,'MyColors',0
db 0x00,0x1e,'FirmwareRevision',0
db 0x00,0x23,'Categories',0
db 0x00,0x24,'FaceDetect1',0
db 0x00,0x25,'FaceDetect2',0
db 0x00,0x26,'CanonAFInfo2',0
db 0x00,0x27,'ContrastInfo',0
db 0x00,0x28,'ImageUniqueID',0
db 0x00,0x2f,'FaceDetect3',0
db 0x00,0x35,'TimeInfo',0
db 0x00,0x81,'RawDataOffset',0
db 0x00,0x83,'OriginalDecisionDataOffset',0
db 0x00,0x90,'CustomFunctions1D',0
db 0x00,0x91,'PersonalFunctions',0
db 0x00,0x92,'PersonalFunctionValues',0
db 0x00,0x93,'CanonFileInfo',0
db 0x00,0x94,'AFPointsInFocus1D',0
db 0x00,0x95,'LensModel',0
db 0x00,0x96,'SerialInfo ',0
db 0x00,0x97,'DustRemovalData',0
db 0x00,0x98,'CropInfo',0
db 0x00,0x99,'CustomFunctions2',0
db 0x00,0x9a,'AspectInfo',0
db 0x00,0xa0,'ProcessingInfo',0
db 0x00,0xa1,'ToneCurveTable',0
db 0x00,0xa2,'SharpnessTable',0
db 0x00,0xa3,'SharpnessFreqTable',0
db 0x00,0xa4,'WhiteBalanceTable',0
db 0x00,0xa9,'ColorBalance',0
db 0x00,0xaa,'MeasuredColor',0
db 0x00,0xae,'ColorTemperature',0
db 0x00,0xb0,'CanonFlags',0
db 0x00,0xb1,'ModifiedInfo',0
db 0x00,0xb2,'ToneCurveMatching',0
db 0x00,0xb3,'WhiteBalanceMatching',0
db 0x00,0xb4,'ColorSpace',0
db 0x00,0xb6,'PreviewImageInfo',0
db 0x00,0xd0,'VRDOffset',0
db 0x00,0xe0,'SensorInfo',0
db 0x40,0x01,'ColorData',0
db 0x40,0x02,'CRWParam?',0
db 0x40,0x03,'ColorInfo',0
db 0x40,0x05,'Flavor?',0
db 0x40,0x08,'BlackLevel?',0
db 0x40,0x10,'CustomPictureStyleFileName',0
db 0x40,0x13,'AFMicroAdj',0
db 0x40,0x15,'VignettingCorr ',0
db 0x40,0x16,'VignettingCorr2',0
db 0x40,0x18,'LightingOpt',0
db 0x40,0x19,'LensInfo',0
db 0x40,0x20,'AmbienceInfo',0
db 0x40,0x24,'FilterInfo',0

dd 0

;данные app2 для Samsung
align 4
gr_927c_Sa:
db 0x00,0x01,'MakerNoteVersion',0	 
db 0x00,0x02,'DeviceType',0 
db 0x00,0x03,'SamsungModelID',0
db 0x00,0x21,'PictureWizard',0
db 0x00,0x30,'LocalLocationName',0
db 0x00,0x31,'LocationName',0
db 0x00,0x35,'PreviewIFD',0
db 0x00,0x43,'CameraTemperature',0
db 0x00,0x45,'RawCompressionMode',0
db 0x01,0x00,'FaceDetect',0
db 0x01,0x20,'FaceRecognition',0
db 0x01,0x23,'FaceName',0
db 0xa0,0x01,'FirmwareName',0
db 0xa0,0x03,'LensType',0
db 0xa0,0x04,'LensFirmware',0
db 0xa0,0x05,'InternalLensSerialNumber',0
db 0xa0,0x10,'SensorAreas',0
db 0xa0,0x11,'ColorSpace',0
db 0xa0,0x12,'SmartRange',0
db 0xa0,0x13,'ExposureCompensation',0
db 0xa0,0x14,'ISO',0
db 0xa0,0x18,'ExposureTime',0
db 0xa0,0x19,'FNumber',0
db 0xa0,0x1a,'FocalLengthIn35mmFormat',0
db 0xa0,0x20,'EncryptionKey',0
db 0xa0,0x21,'WB_RGGBLevelsUncorrected',0
db 0xa0,0x22,'WB_RGGBLevelsAuto',0
db 0xa0,0x23,'WB_RGGBLevelsIlluminator1',0
db 0xa0,0x24,'WB_RGGBLevelsIlluminator2',0
db 0xa0,0x28,'WB_RGGBLevelsBlack',0
db 0xa0,0x30,'ColorMatrix',0
db 0xa0,0x31,'ColorMatrixSRGB',0
db 0xa0,0x32,'ColorMatrixAdobeRGB',0
db 0xa0,0x33,'CbCrMatrixDefault',0
db 0xa0,0x34,'CbCrMatrix',0
db 0xa0,0x35,'CbCrGainDefault',0
db 0xa0,0x36,'CbCrGain',0
db 0xa0,0x40,'ToneCurveSRGBDefault',0
db 0xa0,0x41,'ToneCurveAdobeRGBDefault',0
db 0xa0,0x42,'ToneCurveSRGB',0
db 0xa0,0x43,'ToneCurveAdobeRGB',0
db 0xa0,0x48,'RawData?',0
db 0xa0,0x50,'Distortion?',0
db 0xa0,0x51,'ChromaticAberration?',0
db 0xa0,0x52,'Vignetting?',0
db 0xa0,0x53,'VignettingCorrection?',0
db 0xa0,0x54,'VignettingSetting?',0

dd 0

;input:
; bof - указатель на начало файла
; app1 - указатель для заполнения exif.app1
;output:
; app1 - указатель на начало exif.app1 (или 0 если не найдено или формат файла не поддерживается)
align 4
proc exif_get_app1 uses eax ebx edi, bof:dword, app1:dword
	mov eax,[bof]
	mov edi,[app1]

	;файл в формате jpg?
	cmp word[eax],0xd8ff
	jne .no_exif
	add eax,2

	;файл содержит exif.app0?
	cmp word[eax],0xe0ff
	jne @f
		add eax,2
		movzx ebx,word[eax]
		ror bx,8 ;всегда ли так надо?
		add eax,ebx
	@@:

	;файл содержит exif.app1?
	cmp word[eax],0xe1ff
	jne .no_exif

	xor ebx,ebx
	cmp word[eax+10],'II'
	je @f
		inc ebx ;if 'MM' edx=1
	@@:
	mov word[edi+offs_m_or_i],bx ;способ выравнивания чисел
	mov dword[edi+offs_id_gr],0 ;id групы и производителя, который всегда 0 для app1
	add eax,18
	mov [edi],eax
	sub eax,8
	mov [edi+4],eax

	jmp @f
	.no_exif:
		mov dword[edi],0
	@@:
	ret
endp

;input:
; app1 - указатель на начало exif.app1
; num - порядковый номер тега (начинается с 1)
; txt - указатель на текст, куда будет записано значение
; t_max - максимальный размер текста
;output:
; txt - заполняется текстом в виде "параметр: значение", если не найдено то пустая строка
align 4
proc exif_get_tag, app1:dword, num:dword, txt:dword, t_max:dword
pushad
	mov eax,[app1]
	mov edi,[txt]
	mov ecx,[num]

	xor edx,edx
	mov byte[edi],dl
	cmp [eax],edx
	je .end_f ;если не найден указатель на начало exif.app1
	cmp ecx,edx
	jle .end_f ;если порядковый номер тега <= 0

	movzx edx,word[eax+offs_m_or_i] ;if 'MM' edx=1

	;проверяем число тегов
	mov eax,[eax]
	movzx ebx,word[eax]
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp ecx,ebx
	jg .end_f ;если номер тега больше чем их есть в файле

	;переходим на заданный тег
	dec ecx
	imul ecx,tag_size
	add eax,offs_tag_0
	add eax,ecx

	stdcall read_tag_value,[app1],[t_max]

	.end_f:
popad
	ret
endp

;input:
; app1 - указатель на начало exif.app1
; id - идентификатор тега значение которого нужно найти
; txt - указатель на текст, куда будет записано значение
; t_max - максимальный размер текста
;output:
; txt - заполняется текстом в виде "параметр: значение", если не найдено то пустая строка
align 4
proc exif_get_tag_id, app1:dword, id:dword, txt:dword, t_max:dword
pushad
	mov eax,[app1]
	mov edi,[txt]

	xor edx,edx
	mov byte[edi],dl
	cmp [eax],edx
	je .end_f ;если не найден указатель на начало exif.app1

	mov ebx,[id]
	movzx edx,word[eax+offs_m_or_i] ;if 'MM' edx=1
	bt edx,0
	jnc @f
		ror bx,8
	@@:

	;берем число тегов
	mov eax,[eax]
	movzx ecx,word[eax]
	bt edx,0
	jnc @f
		ror cx,8
	@@:
	;в ecx - число тегов

	;ищем заданный тег
	add eax,offs_tag_0
	.cycle_0:
		cmp word[eax],bx ;word[eax+0] - код тега
		je @f
		add eax,tag_size
		loop .cycle_0
	jmp .end_f
	@@:
		stdcall read_tag_value,[app1],[t_max]
	.end_f:
popad
	ret
endp

;input:
; app1 - указатель на exif.app1 или на exif.app1.child
; child - указатель для заполнения начала дочерних тегов exif.app1.child
; c_tag - тег для которого делается поиск дочерних
;output:
; child - указатель на начало дочерних тегов
align 4
proc exif_get_child, app1:dword, child:dword , c_tag:dword
pushad
	mov eax,[app1]
	mov edi,[child]

	xor edx,edx
	cmp [eax],edx
	je .no_found ;если не найден указатель на начало exif.app1

	movzx edx,word[eax+offs_m_or_i] ;if 'MM' edx=1

	;начало поиска
	mov ebx,[c_tag]
	bt edx,0
	jnc @f
		ror bx,8
	@@:

	;проверяем число тегов
	mov eax,[eax]
	movzx ecx,word[eax]
	bt edx,0
	jnc @f
		ror cx,8
	@@:
	cmp ecx,1
	jl .no_found ;если число тегов <1

	;переходим на 1-й тег
	add eax,offs_tag_0
	@@:
		cmp word[eax],bx
		je @f
		add eax,tag_size
		loop @b
	jmp .no_found ;если не найдено
	@@: ;если найдено
		mov ebx,dword[eax+8]
		bt edx,0
		jnc @f
			bswap ebx
		@@:
		mov eax,[app1]
		add ebx,[eax+4]
		mov dword[edi],ebx
		m2m dword[edi+4],dword[eax+4]
		ror edx,16
		mov dx,word[c_tag]
		ror edx,16
		mov dword[edi+offs_m_or_i],edx
		m2m word[edi+offs_id_gr_mak],word[eax+offs_id_gr_mak]

	jmp .end_f
	.no_found:
		mov dword[edi],0
	.end_f:
popad
	ret
endp

;output:
; app2 - указатель на начало exif.app2 (или 0 если не найдено или формат файла не поддерживается)
align 4
proc exif_get_app2, app1:dword, app2:dword
pushad
	mov edi,[app2]
	mov eax,[app1]
	xor edx,edx
	cmp [eax],edx
	je .no_suport ;если не найден указатель на начало exif.app1

	movzx edx,word[eax+offs_m_or_i] ;if 'MM' edx=1

	;начало поиска производителя камеры
	mov ebx,0x010f
	bt edx,0
	jnc @f
		ror bx,8
	@@:

	;проверяем число тегов
	mov eax,[eax]
	movzx ecx,word[eax]
	bt edx,0
	jnc @f
		ror cx,8
	@@:
	cmp ecx,1
	jl .no_suport ;если число тегов <1

	;переходим на 1-й тег
	add eax,offs_tag_0
	@@:
		cmp word[eax],bx
		je @f
		add eax,tag_size
		loop @b
	jmp .no_suport ;если не найдено
	@@: ;если найдено	
		mov ebx,dword[eax+4]
		bt edx,0
		jnc @f
			bswap ebx
		@@:
		cmp ebx,4
		jle .no_suport ;название производителя меньше 4 символов, не поддержиавается

		mov ebx,dword[eax+8]
		bt edx,0
		jnc @f
			bswap ebx
		@@:
	
	;проверка поддерживаемых производителей
	mov eax,[app1]
	add ebx,[eax+4]
	cmp dword[ebx],'Cano'
	je .suport
	cmp dword[ebx],'NIKO'
	je .suport
	cmp dword[ebx],'Pana'
	je .suport
	cmp dword[ebx],'SAMS'
	je .suport
	cmp dword[ebx],'sams'
	je .suport

	;все остальные не поддерживаются
	jmp .no_suport

	.suport:
	;находим тег 0x8769 (расширенные данные по Exif)
	stdcall exif_get_child, eax,edi,0x8769
	cmp dword[edi],0
	je .no_suport
	;находим тег 0x927c (данные Maker по камере)
	stdcall exif_get_child, edi,edi, 0x927c
	cmp dword[edi],0
	je .no_suport

	cmp dword[ebx],'NIKO'
	jne @f
		;for Nikon
		add dword[edi],18
		mov eax,dword[edi]
		sub eax,8
		mov dword[edi+4],eax
		mov word[edi+offs_id_gr_mak],'Ni'
		jmp .end_f
	@@:
	cmp dword[ebx],'Pana'
	jne @f
		;for Panasonic
		add dword[edi],12
		mov word[edi+offs_id_gr_mak],'Pa'
		jmp .end_f
	@@:
	cmp dword[ebx],'Cano'
	jne @f
		;for Canon
		mov word[edi+offs_id_gr_mak],'Ca'
		jmp .end_f
	@@:

	cmp dword[ebx],'SAMS'
	je @f
	cmp dword[ebx],'sams'
	je @f
		jmp .end_f
	@@:
		;for Samsung
		mov word[edi+offs_id_gr_mak],'Sa'
		jmp .end_f
	@@:

	.no_suport:
		mov dword[edi],0
	.end_f:
popad
	ret
endp

;description:
; вспомогательная функция для чтения назначений тегов
;input:
; eax - указатель начала тега
; edi - указатель на память для записи текстовой строки
align 4
proc read_tag_value, app1:dword, t_max:dword
	push exif_tag_numbers
	pop esi

	;поиск таблицы для группы тегов
	mov ebx,[app1]
	mov ebx,dword[ebx+offs_id_gr] ;берем идентификатор группы тегов
	@@:
		cmp dword[esi],ebx
		je .set_table
		add esi,8
		cmp esi,exif_tag_numbers.end
		jge .tag_unknown ;тег не опознан (не найдена таблица группы тегов)
		jmp @b
	.set_table:
		mov esi,dword[esi+4] ;установка адреса таблицы в esi

	;поиск названия тега в таблице
	.next_tag:
	mov bx,word[esi]
	cmp bx,0
	jne @f
		cmp dword[esi],0
		jne @f
		jmp .tag_unknown ;тег не опознан (в группе не найден тег)
	@@:
	bt edx,0
	jc @f
		ror bx,8
	@@:
	cmp word[eax],bx
	je .found
	inc esi
	@@:
		inc esi
		cmp byte[esi],0
		jne @b
	inc esi
	jmp .next_tag
	.found:

	;копируем строку с названием тега
	add esi,2
	stdcall str_n_cat,edi,esi,[t_max]

	jmp .start_read
	.tag_unknown:
		;если тег не найден ставим его код вместо названия
		movzx ebx,word[eax]
		bt edx,0
		jnc @f
			ror bx,8
		@@:
		stdcall hex_in_str, edi, ebx,4
		mov byte[edi+4],0
	.start_read:

	;читаем информацию в теге
	mov bx,tag_format_ui1b
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_01
		stdcall str_n_cat,edi,txt_dp,[t_max]
		call get_tag_data_size
		cmp ebx,4
		jg .over4b_01
		cmp ebx,1
		je @f
			or edx,2 ;array data
			mov ecx,dword[eax+8]
			mov dh,bl
		@@:
			;если 1 одно байтовое число
			movzx ebx,byte[eax+8]
			stdcall str_len,edi
			add edi,eax
			mov eax,ebx
			stdcall convert_int_to_str, [t_max]
			bt edx,1
			jnc .end_f
			@@:
				;если от 2 до 4 одно байтовых чисел
				dec dh
				cmp dh,0
				je .end_f
				shr ecx,8
				stdcall str_n_cat,edi,txt_zap,[t_max]
				stdcall str_len,edi
				add edi,eax
				movzx eax,cl
				stdcall convert_int_to_str, [t_max]
				jmp @b
		.over4b_01:
			;...
		jmp .end_f
	.tag_01:

	mov bx,tag_format_text
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_02
		stdcall str_n_cat,edi,txt_dp,[t_max]
		call get_tag_data_size ;проверяем длинну строки
		cmp ebx,4
		jg @f
			;если строка помещается в 4 символа
			mov esi,eax
			add esi,8
			stdcall str_n_cat,edi,esi,[t_max]
			jmp .end_f
		;если строка не помещается в 4 символа
		@@:
		mov esi,dword[eax+8]
		bt edx,0
		jnc @f
			bswap esi
		@@:
		mov eax,[app1]
		mov eax,[eax+4]
		add esi,eax
		stdcall str_n_cat,edi,esi,[t_max]
		jmp .end_f
	.tag_02:

	mov bx,tag_format_ui2b
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_03
		stdcall str_n_cat,edi,txt_dp,[t_max]
		call get_tag_data_size
		cmp ebx,2
		jg .over4b_03
		jne @f
			;если два 2 байтовых числа
			or edx,2 ;array data
			movzx ecx,word[eax+10]
			bt edx,0
			jnc @f
				ror cx,8
			@@:
			;если одно 2 байтовое число
			movzx ebx,word[eax+8]
			bt edx,0
			jnc @f
				ror bx,8
			@@:
			stdcall str_len,edi
			add edi,eax
			mov eax,ebx
			stdcall convert_int_to_str, [t_max]
			bt edx,1 ;array ?
			jnc .end_f
			;добавляем 2-е число
			stdcall str_n_cat,edi,txt_zap,[t_max]
			stdcall str_len,edi
			add edi,eax
			mov eax,ecx
			stdcall convert_int_to_str, [t_max]
			jmp .end_f
		.over4b_03:
			mov ecx,[t_max]
			mov esi,dword[eax+8]
			bt edx,0
			jnc @f
				bswap esi
			@@:
			mov eax,[app1]
			mov eax,[eax+4]
			add esi,eax
			
			;берем число
			.array_03:
			stdcall str_len,edi
			cmp ecx,eax
			jle .end_f ;если не хватило строки
			add edi,eax
			sub ecx,eax
			movzx eax,word[esi]
			bt edx,0
			jnc @f
				ror ax,8
			@@:
			stdcall convert_int_to_str,ecx

			dec ebx
			cmp ebx,0
			je .end_f

			;добавляем запятую
			stdcall str_n_cat,edi,txt_zap,ecx
			add esi,2
			jmp .array_03
	.tag_03:

	mov bx,tag_format_ui4b
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_04
		stdcall str_n_cat,edi,txt_dp,[t_max]
		call get_tag_data_size
		cmp ebx,1
		jg .over4b_04
			;если одно 4 байтовое число
			mov ebx,dword[eax+8]
			bt edx,0
			jnc @f
				bswap ebx
			@@:
			stdcall str_len,edi
			add edi,eax
			mov eax,ebx
			stdcall convert_int_to_str, [t_max]
		.over4b_04:
			;...
		jmp .end_f
	.tag_04:

	mov bx,tag_format_urb
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_05
		stdcall str_n_cat,edi,txt_dp,[t_max]
		;call get_tag_data_size
		;cmp ebx,1
		;jg .over4b_05
			mov ebx,dword[eax+8]
			bt edx,0
			jnc @f
				bswap ebx
			@@:
			stdcall str_len,edi
			add edi,eax
			mov eax,[app1]
			mov eax,[eax+4]
			add ebx,eax
			mov eax,[ebx]
			bt edx,0
			jnc @f
				bswap eax
			@@:
			stdcall convert_int_to_str, [t_max] ;ставим 1-е число
			stdcall str_n_cat,edi,txt_div,[t_max] ;ставим знак деления
			stdcall str_len,edi
			add edi,eax
			mov eax,[ebx+4]
			bt edx,0
			jnc @f
				bswap eax
			@@:
			stdcall convert_int_to_str, [t_max] ;ставим 2-е число
		;.over4b_05:
			;...
		jmp .end_f
	.tag_05:

	mov bx,tag_format_si2b
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_08
		stdcall str_n_cat,edi,txt_dp,[t_max]
		call get_tag_data_size
		cmp ebx,2
		jg .over4b_08
		jne @f
			;если два 2 байтовых числа
			or edx,2 ;array data
			movzx ecx,word[eax+10]
			bt edx,0
			jnc @f
				ror cx,8
			@@:		
			;если одно 2 байтовое число
			movzx ebx,word[eax+8]
			bt edx,0
			jnc @f
				ror bx,8
			@@:
			stdcall str_len,edi
			add edi,eax
			bt bx,15
			jnc @f
				mov byte[edi],'-'
				inc edi
				neg bx
				inc bx
			@@:
			mov eax,ebx
			stdcall convert_int_to_str, [t_max]
			bt edx,1 ;array ?
			jnc .end_f
			;добавляем 2-е число
			stdcall str_n_cat,edi,txt_zap,[t_max]
			stdcall str_len,edi
			add edi,eax
			bt bx,15
			jnc @f
				mov byte[edi],'-'
				inc edi
				neg bx
				inc bx
			@@:
			mov eax,ecx
			stdcall convert_int_to_str, [t_max]
			jmp .end_f
		.over4b_08:
			mov ecx,[t_max]
			mov esi,dword[eax+8]
			bt edx,0
			jnc @f
				bswap esi
			@@:
			mov eax,[app1]
			mov eax,[eax+4]
			add esi,eax
			
			;берем число
			.array_08:
			stdcall str_len,edi
			cmp ecx,eax
			jle .end_f ;если не хватило строки
			add edi,eax
			sub ecx,eax
			movzx eax,word[esi]
			bt edx,0
			jnc @f
				ror ax,8
			@@:
			;смотрим на знак +|-
			bt ax,15
			jnc @f
				mov byte[edi],'-'
				inc edi
				neg ax
				inc ax
			@@:
			stdcall convert_int_to_str,ecx

			dec ebx
			cmp ebx,0
			je .end_f

			;добавляем запятую
			stdcall str_n_cat,edi,txt_zap,ecx
			add esi,2
			jmp .array_08
	.tag_08:

	mov bx,tag_format_si4b
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_09
		stdcall str_n_cat,edi,txt_dp,[t_max]
		call get_tag_data_size
		cmp ebx,1
		jg .over4b_09
			;если одно 4 байтовое число
			mov ebx,dword[eax+8]
			bt edx,0
			jnc @f
				bswap ebx
			@@:
			stdcall str_len,edi
			add edi,eax
			bt ebx,31
			jnc @f
				mov byte[edi],'-'
				inc edi
				neg ebx
				inc ebx
			@@:
			mov eax,ebx
			stdcall convert_int_to_str, [t_max]
		.over4b_09:
			;...
		jmp .end_f
	.tag_09:

	.end_f:
	ret
endp

;input:
; eax - tag pointer
; edx - 1 if 'MM', 0 if 'II'
;output:
; ebx - data size
align 4
get_tag_data_size:
	mov ebx,dword[eax+4]
	bt edx,0
	jnc @f
		bswap ebx
	@@:
	ret

align 4
proc str_n_cat uses eax ecx edi esi, str1:dword, str2:dword, n:dword
	mov esi,dword[str2]
	mov ecx,dword[n]
	mov edi,dword[str1]
	stdcall str_len,edi
	add edi,eax
	cld
	repne movsb
	mov byte[edi],0
	ret
endp

;output:
; eax = strlen
align 4
proc str_len, str1:dword
	mov eax,[str1]
	@@:
		cmp byte[eax],0
		je @f
		inc eax
		jmp @b
	@@:
	sub eax,[str1]
	ret
endp

align 4
proc hex_in_str, buf:dword,val:dword,zif:dword
	pushad
		mov edi,dword[buf]
		mov ecx,dword[zif]
		add edi,ecx
		dec edi
		mov ebx,dword[val]

		.cycle:
			mov al,bl
			and al,0xf
			cmp al,10
			jl @f
				add al,'a'-'0'-10
			@@:
			add al,'0'
			mov byte[edi],al
			dec edi
			shr ebx,4
		loop .cycle

	popad
	ret
endp

;input:
; eax - число
; edi - буфер для строки
; len - длинна буфера
;output:
align 4
proc convert_int_to_str, len:dword
pushad
	mov esi,[len]
	add esi,edi
	dec esi
	call .str
popad
	ret
endp

align 4
.str:
	mov ecx,0x0a ;задается система счисления изменяются регистры ebx,eax,ecx,edx входные параметры eax - число
	;преревод числа в ASCII строку взодные данные ecx=система счисленя edi адрес куда записывать, будем строку, причем конец переменной 
	cmp eax,ecx ;сравнить если в eax меньше чем в ecx то перейти на @@-1 т.е. на pop eax
	jb @f
		xor edx,edx ;очистить edx
		div ecx   ;разделить - остаток в edx
		push edx  ;положить в стек
		call .str ;перейти на саму себя т.е. вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
		pop eax
	@@: ;cmp al,10 ;проверить не меньше ли значение в al чем 10 (для системы счисленя 10 данная команда - лишная))
	cmp edi,esi
	jge @f
		or al,0x30 ;данная команда короче чем две выше
		stosb      ;записать элемент из регистра al в ячеку памяти es:edi
		mov byte[edi],0 ;в конец строки ставим 0, что-бы не вылазил мусор
	@@:
	ret        ;пока в стеке храниться кол-во вызовов то столько раз мы и будем вызываться



align 16
EXPORTS:
	dd sz_exif_get_app1, exif_get_app1
	dd sz_exif_get_app2, exif_get_app2
	dd sz_exif_get_tag, exif_get_tag
	dd sz_exif_get_tag_id, exif_get_tag_id
	dd sz_exif_get_child, exif_get_child
	dd 0,0
	sz_exif_get_app1 db 'exif_get_app1',0
	sz_exif_get_app2 db 'exif_get_app2',0
	sz_exif_get_tag db 'exif_get_tag',0
	sz_exif_get_tag_id db 'exif_get_tag_id',0
	sz_exif_get_child db 'exif_get_child',0

