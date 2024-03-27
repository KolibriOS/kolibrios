#ifndef KOLIBRI_COLORS_H
#define KOLIBRI_COLORS_H
/*
  * +0: dword: frames - ���� �����
  * +4: dword: grab - ���� ���������
  * +8: dword: grab_button - ���� ������ �� ������ ���������
  * +12 = +0xC: dword: grab_button_text - ���� ������ �� ������
    �� ������ ���������
  * +16 = +0x10: dword: grab_text - ���� ������ �� ���������
  * +20 = +0x14: dword: work - ���� ������� �������
  * +24 = +0x18: dword: work_button - ���� ������ � ������� �������
  * +28 = +0x1C: dword: work_button_text - ���� ������ �� ������
    � ������� �������
  * +32 = +0x20: dword: work_text - ���� ������ � ������� �������
  * +36 = +0x24: dword: work_graph - ���� ������� � ������� �������
*/

typedef struct {
  unsigned int color_frame_area; // 0 ���� �����
  unsigned int color_grab_bar; // 4
  unsigned int color_grab_bar_button; // 8
  unsigned int color_grab_button_text; // 12
  unsigned int color_grab_text; // 16
  unsigned int color_work_area; // 20
  unsigned int color_work_button; // 24
  unsigned int color_work_button_text; // 28
  unsigned int color_work_text; // 32
  unsigned int color_work_graph; // 36
} kolibri_system_colors;

kolibri_system_colors kolibri_color_table;

/// @brief Get system colors table
/// @param color_table Pointer to table
void kolibri_get_system_colors(kolibri_system_colors *color_table)
{
  __asm__ volatile ("int $0x40"
		    :
		    :"a"(48),"b"(3),"c"(color_table),"d"(40)
		    );

  /* color_table should point to the system color table */
}

#endif /* KOLIBRI_COLORS_H */
