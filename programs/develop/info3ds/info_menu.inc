; Language support for locales: ru_RU (CP866), en_US.

if lang eq ru_RU

sz_main_menu_View  db '���', 0
sz_main_menu_Veiw_Vertexes db '���設� ���.', 0
sz_main_menu_Veiw_Faces db '��ઠ�� �࠭� ���.', 0
sz_main_menu_Veiw_Faces_Fill db '������ �࠭� ���.', 0
sz_main_menu_Veiw_Light db '���� ���./�몫.', 0
sz_main_menu_Veiw_Smooth db '����������� ���./�몫.', 0
sz_main_menu_Veiw_Reset db '���� ����஥�', 0

if version_edit eq 0
sz_main_menu_Veiw_Faces_Mat db '�����梥�� �࠭� ���.', 0
else
sz_main_menu_Vertexes db '���設�', 0
sz_main_menu_Vertexes_Select db '�뤥���� ���設�', 0
sz_main_menu_Vertexes_Deselect db '�⬥���� ��� �뤥�����', 0
sz_main_menu_Average_x db '�।��� x',0
sz_main_menu_Average_y db '�।��� y',0
sz_main_menu_Average_z db '�।��� z',0
end if

else ; Default to en_US

sz_main_menu_View  db 'View', 0
sz_main_menu_Veiw_Vertexes db 'Vertexes on', 0
sz_main_menu_Veiw_Faces db 'Edges on', 0
sz_main_menu_Veiw_Faces_Fill db 'Faces on', 0
sz_main_menu_Veiw_Light db 'Light on/off', 0
sz_main_menu_Veiw_Smooth db 'Smooth on/off', 0
sz_main_menu_Veiw_Reset db 'Reset settings', 0

if version_edit eq 0
sz_main_menu_Veiw_Faces_Mat db 'Diferent color faces on', 0
else
sz_main_menu_Vertexes db 'Vertexes', 0
sz_main_menu_Vertexes_Select db 'Select vertex', 0
sz_main_menu_Vertexes_Deselect db 'Deselect all', 0
sz_main_menu_Average_x db 'Average x',0
sz_main_menu_Average_y db 'Average y',0
sz_main_menu_Average_z db 'Average z',0
end if

end if

main_menu dd 0
main_menu_file dd 0
main_menu_view dd 0
main_menu_vertexes dd 0

KMENUITEM_NORMAL equ 0
KMENUITEM_SUBMENU equ 1
KMENUITEM_SEPARATOR equ 2
