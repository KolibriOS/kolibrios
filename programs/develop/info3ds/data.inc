;
; ����� ᮡ࠭� ����� �� ������ *.3ds
;

; Language support for locales: ru_RU (CP866), en_US.

MAX_FILE_LEVEL equ 20 ;���ᨬ���� �஢��� ���������� ������ ��� �������

sizeof.block_3ds equ 9
can_save db 0 ;�������� �� 䠩�

macro block_3ds id,icon,par,caption
{
	dw id ;+0 �����䨪��� ����� � 䠩�� 3ds
	dw icon ;+2 ����� ������ ��� �����
	db par ;+4 ᮤ�ন� �� ���� �������� (0-�� 1-���)
	dd caption+0 ;+5 ���ᠭ�� �����
}

CHUNK_MAIN equ 0x4D4D ; [-] �業�
CHUNK_ambient_color	  equ 0x2100 ; [-] ambient color
  CHUNK_OBJMESH 	  equ 0x3D3D ; [-] ����� ��ꥪ⮢
    CHUNK_OBJBLOCK	  equ 0x4000 ; [+] ��ꥪ�
      CHUNK_TRIMESH	  equ 0x4100 ; [-] trimesh-��ꥪ�
	CHUNK_VERTLIST	  equ 0x4110 ; [+] ᯨ᮪ ���設
	CHUNK_FACELIST	  equ 0x4120 ; [+] ᯨ᮪ �࠭��
	CHUNK_FACEMAT	  equ 0x4130 ; [+] ���ਠ�� �࠭��
	CHUNK_MAPLIST	  equ 0x4140 ; [+] ⥪����� ���न����
	CHUNK_TRMATRIX	  equ 0x4160 ; [+] ����� ��ॢ���
      CHUNK_CAMERA	  equ 0x4700 ; [+] ��ꥪ�-�����
  CHUNK_MATERIAL	  equ 0xAFFF ; [-] ���ਠ�
    CHUNK_MATNAME	  equ 0xA000 ; [+] �������� ���ਠ��
    CHUNK_TEXTURE	  equ 0xA200 ; [-] ⥪���� ���ਠ��
      CHUNK_MAPFILE	  equ 0xA300 ; [+] ��� 䠩�� ⥪�����
  CHUNK_KEYFRAMER	  equ 0xB000 ; [-] ���ଠ�� �� �����樨
  CHUNK_TRACKINFO	  equ 0xB002 ; [-] ��������� ��ꥪ�
    CHUNK_TRACKOBJNAME	  equ 0xB010 ; [+] �������� �⮣� ��ꥪ�
    CHUNK_TRACKPIVOT	  equ 0xB013 ; [+] 業�� ��饭�� ��ꥪ�
    CHUNK_TRACKPOS	  equ 0xB020 ; [+] �ࠥ���� ��ꥪ�
    CHUNK_TRACKROTATE	  equ 0xB021 ; [+] �ࠥ���� ��饭�� ��ꥪ�
  CHUNK_TRACKCAMERA	  equ 0xB003 ; [-] ��������� ������
    CHUNK_TRACKFOV	  equ 0xB023 ; [+] ��������� fov ������
    CHUNK_TRACKROLL	  equ 0xB024 ; [+] ��������� roll ������
  CHUNK_TRACKCAMTGT	  equ 0xB004 ; [-] ��������� "楫�" ������

;����� ᮤ�ঠ� ���� �����, �⬥祭�� ���ᮬ, ��⠫�� �����
; ����� ���� �� ���������

align 4
type_bloks:
block_3ds 0x0002,5,1,txt_0002
block_3ds 0x3d3e,5,1 ; [+] mesh version
block_3ds 0x0011,4,1,txt_0011
block_3ds 0x0100,3,1,txt_0100
block_3ds CHUNK_ambient_color,3,0,txt_2100 ; [-] ambient color
block_3ds CHUNK_OBJMESH, 3,0,txt_3d3d
block_3ds CHUNK_OBJBLOCK,11,1,txt_4000
block_3ds CHUNK_TRIMESH, 3,0,txt_4100
block_3ds CHUNK_VERTLIST,8,1,txt_4110
block_3ds CHUNK_FACELIST,3,1,txt_4120
block_3ds CHUNK_FACEMAT,10,1,txt_4130
block_3ds CHUNK_MAPLIST, 3,1,txt_4140
block_3ds CHUNK_TRMATRIX,3,1,txt_4160
block_3ds 0x4600,3,1,txt_4600
block_3ds CHUNK_CAMERA,  3,1,txt_4700
block_3ds CHUNK_MATERIAL,10,0,txt_afff
block_3ds CHUNK_MATNAME, 3,1,txt_a000
block_3ds 0xa010,4,0,txt_a010
block_3ds 0xa020,4,0,txt_a020
block_3ds 0xa030,4,0,txt_a030
block_3ds 0xa100,5,1,txt_a100
block_3ds CHUNK_TEXTURE, 3,0,txt_a200
block_3ds CHUNK_MAPFILE, 6,1,txt_a300
block_3ds CHUNK_KEYFRAMER,   7,0,txt_b000
block_3ds CHUNK_TRACKINFO,   3,0,txt_b002
block_3ds CHUNK_TRACKOBJNAME,3,1,txt_b010
block_3ds 0xb011,3,1,txt_b011
block_3ds CHUNK_TRACKPIVOT,  3,1,txt_b013
block_3ds 0xb014,3,1,txt_b014
block_3ds CHUNK_TRACKPOS,    3,1,txt_b020
block_3ds CHUNK_TRACKROTATE, 3,1,txt_b021
block_3ds CHUNK_TRACKCAMERA, 7,0,txt_b003
block_3ds CHUNK_TRACKFOV,    7,1,txt_b023
block_3ds CHUNK_TRACKROLL,   7,1,txt_b024
block_3ds CHUNK_TRACKCAMTGT, 7,0,txt_b004

;--- ����� �� �� ��� ������ ᬮ� ���� ���ᠭ�� �� ���᪮� �몥
block_3ds 0x4111, 3,1,txt_4111
block_3ds 0x4150, 3,1,txt_4150
block_3ds 0x4165, 3,1,txt_4165
block_3ds 0x4610, 3,1,txt_4610
block_3ds 0x4627, 3,1,txt_4627
block_3ds 0x4630, 3,1,txt_4630
block_3ds 0x4641, 3,1,txt_4641
block_3ds 0x4650, 3,1,txt_4650
block_3ds 0x4651, 3,1,txt_4651
block_3ds 0x4652, 3,1,txt_4652
block_3ds 0x4653, 3,1,txt_4653
block_3ds 0x4656, 3,1,txt_4656
block_3ds 0x4658, 3,1,txt_4658
block_3ds 0x4620, 3,1,txt_4620
block_3ds 0x4625, 3,1,txt_4625
block_3ds 0x4659, 3,1,txt_4659
block_3ds 0x465a, 3,1,txt_465a
block_3ds 0x465b, 3,1,txt_465b
block_3ds 0xa230, 3,1,txt_a230
block_3ds 0xa220, 3,1,txt_a220
block_3ds 0xa351, 3,1,txt_a351
block_3ds 0xb007, 3,1,txt_b007
block_3ds 0xb008, 3,1,txt_b008
block_3ds 0xb022, 3,1,txt_b022
block_3ds 0xb030, 3,1,txt_b030
block_3ds 0xa040, 3,1,txt_a040
block_3ds 0xa041, 3,1,txt_a041
block_3ds 0xa050, 3,1,txt_a050
block_3ds 0xa052, 3,1,txt_a052
block_3ds 0xa053, 3,1,txt_a053
block_3ds 0xa081, 3,1,txt_a081
block_3ds 0xa083, 3,1,txt_a083
block_3ds 0xa084, 3,1,txt_a084
block_3ds 0xa085, 3,1,txt_a085
block_3ds 0xa087, 3,1,txt_a087
block_3ds 0xa088, 3,1,txt_a088
block_3ds 0xa08a, 3,1,txt_a08a
block_3ds 0xa08c, 3,1,txt_a08c
block_3ds 0xa08e, 3,1,txt_a08e
.end:

if lang eq ru_RU
txt_0002 db '3ds �����',0
txt_0011 db '���� rgb (���⮢� �ଠ�)',0
;txt_0012 db 'LIN_COLOR_24',0
;txt_0030 db 'Percentage value (int)',0
;txt_0031 db 'Percentage value (float)',0
txt_0100 db 'Master scale',0
txt_2100 db '���� ���㦠�饩 �।�',0
txt_3d3d db '����� ��ꥪ⮢',0
txt_4000 db '��ꥪ� (� ������)',0
txt_4100 db '��㣮��� �⪨',0
txt_4110 db '���᮪ ���設',0
txt_4120 db '���᮪ �࠭��',0
txt_4130 db '���ਠ�� �࠭��',0
txt_4140 db '�������� ���न����',0
txt_4160 db '����� ��ॢ���',0
txt_4600 db '����',0
txt_4700 db '�����',0
txt_a000 db '�������� ���ਠ��',0
txt_a010 db '���㦠�騩 梥�',0
txt_a020 db '����㧭� 梥�',0
txt_a030 db '��ઠ��� 梥�',0
txt_a100 db '��� ���ਠ�� [1=flat 2=gouraud 3=phong 4=metal]',0
txt_a200 db '������� ���ਠ�� 1',0
txt_a300 db '��� 䠩�� ⥪�����',0
;txt_a353 db 'MAT_MAP_TEXBLUR',0
txt_afff db '���ਠ�',0
txt_b000 db '���ଠ�� �� �����樨',0
txt_b002 db '��������� ��ꥪ�',0
;txt_b009 db 'KFCURTIME',0
;txt_b00a db 'KFHDR',0
txt_b010 db '�������� ��ꥪ�',0
txt_b011 db '��� ���������',0
txt_b013 db '����� ��饭�� ��ꥪ�',0
txt_b014 db 'Bound box',0
txt_b020 db '�ࠥ���� ��������',0
txt_b021 db '�ࠥ���� ��饭��',0
txt_b003 db '��������� ������',0
txt_b023 db '��������� fov ������',0
txt_b024 db '��������� roll ������',0
txt_b004 db '��������� "楫�" ������',0

;--- ����� �� �� ��� ������ ᬮ� ���� ���ᠭ�� �� ���᪮� �몥
txt_4111 db '����� ���設',0
txt_4150 db 'Smoothing group list',0
txt_4165 db 'Object color in editor',0
txt_4610 db '���ࠢ����� ᢥ�',0
txt_4627 db 'Spot raytrace',0
txt_4630 db 'Light shadowed',0
txt_4641 db 'Spot shadow map',0
txt_4650 db 'Spot show cone',0
txt_4651 db 'Spot is rectangular',0
txt_4652 db 'Spot overshoot',0
txt_4653 db 'Spot map',0
txt_4656 db 'Spot roll',0
txt_4658 db 'Spot ray trace bias',0
txt_4620 db 'Light off',0
txt_4625 db 'Attenuation on',0
txt_4659 db 'Range start',0
txt_465a db 'Range end',0
txt_465b db 'Multiplier',0
txt_a230 db '�����⪠ ५��',0
txt_a220 db '�����⪠ ��ࠦ����',0
txt_a351 db '��ࠬ���� ࠧ��⪨',0
txt_b007 db '���ଠ�� � ���ࠢ����� �ᢥ饭��',0
txt_b008 db '����� (��砫�� � ������)',0
txt_b022 db '����⠡�஢����',0
txt_b030 db '������ � ����娨',0
txt_a040 db 'Shininess percent',0
txt_a041 db 'Shininess strength percent',0
txt_a050 db 'Transparency percent',0
txt_a052 db 'Transparency falloff percent',0
txt_a053 db 'Reflection blur percent',0
txt_a081 db '2 sided',0
txt_a083 db 'Add trans',0
txt_a084 db 'Self illum',0
txt_a085 db 'Wire frame on',0
txt_a087 db 'Wire thickness',0
txt_a088 db 'Face map',0
txt_a08a db 'In tranc',0
txt_a08c db 'Soften',0
txt_a08e db 'Wire in units',0

txt_not_delete db '�� ���� 㤠���� ��࠭�� ����. �� ���饭.',0

else ; Default to en_US

txt_0002 db '3ds version',0
txt_0011 db 'Color rgb (byte format)',0
txt_0100 db 'Master scale',0
txt_2100 db 'Ambient color',0
txt_3d3d db 'Editor chunk',0
txt_4000 db 'Object (with name)',0
txt_4100 db 'Triangular mesh',0
txt_4110 db 'Vertices list',0
txt_4120 db 'Faces description',0
txt_4130 db 'Faces material',0
txt_4140 db 'Mapping coordinates list',0
txt_4160 db 'Local coordinate system',0
txt_4600 db 'Light',0
txt_4700 db 'Camera',0
txt_a000 db 'Material name',0
txt_a010 db 'Ambient color',0
txt_a020 db 'Diffuse color',0
txt_a030 db 'Specular color',0
txt_a100 db 'Material type [1=flat 2=gouraud 3=phong 4=metal]',0
txt_a200 db 'Texture map 1',0
txt_a300 db 'Mapping filename',0
txt_afff db 'Meterial',0
txt_b000 db 'Keyframer',0
txt_b002 db 'Mesh information',0
txt_b010 db 'Object name',0
txt_b011 db 'Instance name',0
txt_b013 db 'Object pivot point',0
txt_b014 db 'Bound box',0
txt_b020 db 'Position track',0
txt_b021 db 'Rotation track',0
txt_b003 db 'Camera information',0
txt_b023 db 'Fov track',0
txt_b024 db 'Roll track',0
txt_b004 db 'Camera target information',0

;---
txt_4111 db 'Vertex flags',0
txt_4150 db 'Smoothing group list',0
txt_4165 db 'Object color in editor',0
txt_4610 db 'Spotlight',0
txt_4627 db 'Spot raytrace',0
txt_4630 db 'Light shadowed',0
txt_4641 db 'Spot shadow map',0
txt_4650 db 'Spot show cone',0
txt_4651 db 'Spot is rectangular',0
txt_4652 db 'Spot overshoot',0
txt_4653 db 'Spot map',0
txt_4656 db 'Spot roll',0
txt_4658 db 'Spot ray trace bias',0
txt_4620 db 'Light off',0
txt_4625 db 'Attenuation on',0
txt_4659 db 'Range start',0
txt_465a db 'Range end',0
txt_465b db 'Multiplier',0
txt_a230 db 'Bump map',0
txt_a220 db 'Reflection map',0
txt_a351 db 'Mapping parameters',0
txt_b007 db 'Spot light information',0
txt_b008 db 'Frames (Start and End)',0
txt_b022 db 'Scale track',0
txt_b030 db 'Hierarchy position',0
txt_a040 db 'Shininess percent',0
txt_a041 db 'Shininess strength percent',0
txt_a050 db 'Transparency percent',0
txt_a052 db 'Transparency falloff percent',0
txt_a053 db 'Reflection blur percent',0
txt_a081 db '2 sided',0
txt_a083 db 'Add trans',0
txt_a084 db 'Self illum',0
txt_a085 db 'Wire frame on',0
txt_a087 db 'Wire thickness',0
txt_a088 db 'Face map',0
txt_a08a db 'In tranc',0
txt_a08c db 'Soften',0
txt_a08e db 'Wire in units',0

txt_not_delete db 'Cannot delete the selected chunk: it is protected.',0
end if

if lang eq ru_RU
txt_open_3ds db '����� 䠩�:',0
txt_no_3ds: db '������ 䠩� �� � �ଠ� *.3ds ',39
	.zag: rb 8
	db 39,0
txt_3ds_err_sizes db '�������� 䠩� ���०���',0
txt_count db '������⢮',0
txt_3ds_offs:
	db '���饭��: '
	.dig: rb 8
	db '  ������: '
	.siz: rb 8
	db 0
txt_mat_null db '���ਠ�� �� �������',0

else ; Default to en_US
txt_open_3ds db 'Open file:',0
txt_no_3ds: db 'Open file not in format *.3ds ',39
	.zag: rb 8
	db 39,0
txt_3ds_err_sizes db 'File may be corrupted',0
txt_count db 'Count',0
txt_3ds_offs:
	db 'Offset: '
	.dig: rb 8
	db '  Size: '
	.siz: rb 8
	db 0
txt_mat_null db 'No materials found',0
end if
