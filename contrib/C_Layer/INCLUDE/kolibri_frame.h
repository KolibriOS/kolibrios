#ifndef KOLIBRI_FRAME_H
#define KOLIBRI_FRAME_H


enum fr_text_position_t {
	FR_TOP,
	FR_BOTTON
};

/// @brief Flags
typedef enum fr_flags_t
{
    /// @brief specifies whether to display text or not
    /// @note if text != null set auto
    FR_CAPTION = 1,
    /// @brief default
    /// \image html SCR_1.PNG
    FR_DOUBLE = 0,
    /// \image html SCR_2.PNG
    FR_RAISED = 2,
    /// \image html SCR_3.PNG
    FR_SUNKEN = 4,
    /// \image html SCR_4.PNG
    FR_ETCHED = 6,
    /// \image html SCR_5.PNG
    FR_RINGED = 8,
    /// @brief whether to paint the background inside the element or not
    /// @note does not affect the text background
    FR_FILLED = 0x10
};

typedef struct {
    /// @brief not use
	uint32_t type;
	union 
    {   
        /// @brief coord by X + frame width
        uint32_t x_w;
        struct
        {
            /// @brief x coordinate of the upper left corner
            uint16_t x;
            /// @brief frame width
            uint16_t width;
        };
    };
	union 
    {
        /// @brief Y coord + frame height
        uint32_t y_h;
        struct 
        {
            /// @brief y coordinate of the upper left corner
            uint16_t y;
            /// @brief frame height
            uint16_t height;
        };
    };

    /// @brief outer frame color
    color_t ext_col;

    /// @brief inner frame color
    color_t int_col;

    /// @brief Flags, value from enum fr_flags_t
	uint32_t flags;

    /// @brief Pointer to string
    char *text_pointer;

    /// @brief This is a bit flag. If it is set to zero, then the inscription will be at the top of the frame, if 1, then the inscription will be at the bottom of the frame.
    uint32_t text_position;

    /// @brief font and format of the output line.
    uint32_t font_number;

    /// @brief upward shift of the output text.
    uint32_t font_size_y;

    /// @brief color of the output text.
    color_t font_color;

    /// @brief background color for text.
    color_t font_bg_color;
} frame;

/// @brief inilizate drame struct
/// @param f Pointer to frame
/// @param x_w coord by X + frame width
/// @param y_h Y coord + frame height
/// @param ext_col outer frame color
/// @param int_col inner frame color
/// @param text Pointer to string
/// @param text_position This is a bit flag. If it is set to zero, then the inscription will be at the top of the frame, if 1, then the inscription will be at the bottom of the frame.
/// @param fint_color color of the output text.
/// @param font_bgcolor ackground color for text.
/// @param flags Flags, value from enum fr_flags_t
static inline frame* kolibri_frame(frame* f, uint32_t x_w, uint32_t y_h, color_t ext_col, color_t int_col, char *text, enum fr_text_position_t text_position,
                                   color_t font_color, color_t font_bgcolor, enum fr_flags_t flags)
{
    f->type = 0;
    f->x_w = x_w;
    f->y_h = y_h;
    f->ext_col = ext_col;
    f->int_col = int_col;
    f->flags = flags;
    if (text) f->flags |= FR_CAPTION;
    f->text_pointer = text;
    f->text_position = text_position;
    f->font_number = 0;  // 0 == font 6x9, 1==8x16
    f->font_size_y = 9;
    f->font_color = font_color | 0x80000000;
    f->font_bg_color = font_bgcolor;

    return f;
}

/// @brief Create frame struct
/// @param f Pointer to frame
/// @param x_w coord by X + frame width
/// @param y_h Y coord + frame height
/// @param ext_col outer frame color
/// @param int_col inner frame color
/// @param text Pointer to string
/// @param text_position This is a bit flag. If it is set to zero, then the inscription will be at the top of the frame, if 1, then the inscription will be at the bottom of the frame.
/// @param fint_color color of the output text.
/// @param font_bgcolor ackground color for text.
/// @param flags Flags, value from enum fr_flags_t
/// @return Pointer to new frame
static inline frame* kolibri_new_frame(uint32_t x_w, uint32_t y_h, color_t ext_col, color_t int_col, char *text, enum fr_text_position_t text_position,
                                       color_t font_color, color_t font_bgcolor, enum fr_flags_t flags)
{
    frame *new_frame = (frame *)malloc(sizeof(frame));
    return kolibri_frame(new_frame, x_w, y_h, ext_col, int_col, text, text_position, font_color, font_bgcolor, flags);
}

/// @brief Init frame struct, using system colors
/// @param f Pointer to frame
/// @param x_w coord by X + frame width
/// @param y_h Y coord + frame height
/// @param text Pointer to string
/// @return Pointer to frame
static inline frame* kolibri_frame_def(frame* f, uint32_t x_w, uint32_t y_h, char *text)
{
    return kolibri_frame(f, x_w, y_h, 0x00FCFCFC, 0x00DCDCDC, text, FR_TOP, kolibri_color_table.color_work_text, kolibri_color_table.color_work_area, 0);
}

/// @brief Create frame struct, using system colors
/// @param f Pointer to frame
/// @param x_w coord by X + frame width
/// @param y_h Y coord + frame height
/// @param text Pointer to string
/// @return Pointer to new frame
static inline frame* kolibri_new_frame_def(uint32_t x_w, uint32_t y_h, char *text)
{
    return kolibri_new_frame(x_w, y_h, 0x00FCFCFC, 0x00DCDCDC, text, FR_TOP, kolibri_color_table.color_work_text, kolibri_color_table.color_work_area, 0);
}

static inline void gui_add_frame(kolibri_window *wnd, frame* f)
{
    kolibri_window_add_element(wnd, KOLIBRI_FRAME, f);
}

/// @brief Draw frame
/// @param frame Pointer to frame to be draw
extern void (*frame_draw)(frame *frame) __attribute__((__stdcall__));

#endif /* KOLIBRI_FRAME_H */
