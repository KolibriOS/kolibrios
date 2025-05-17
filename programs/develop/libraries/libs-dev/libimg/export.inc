;
; Export functions
;

E_LIB lib_init
E_LIB version
E_LIB img_is_img
E_LIB img_info
E_LIB img_from_file
E_LIB img_to_file
E_LIB img_from_rgb
E_LIB img_to_rgb
E_LIB img_to_rgb2
E_LIB img_decode
E_LIB img_encode ;supported formats: PNG 24 32, BMP 24 32, PNM 1 8g 24
E_LIB img_create
E_LIB img_destroy
E_LIB img_destroy_layer
E_LIB img_count
E_LIB img_lock_bits
E_LIB img_unlock_bits
E_LIB img_flip
E_LIB img_flip_layer
E_LIB img_rotate
E_LIB img_rotate_layer
E_LIB img_draw
E_LIB img_scale
E_LIB img_get_scaled_size
E_LIB img_convert
E_LIB img_blend
E_LIB img_resize_data
E_LIB img_formats_table

purge E_LIB
