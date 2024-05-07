#ifndef KOLIBRI_LIBINI_H
#define KOLIBRI_LIBINI_H

#include <stdint.h>

/// @brief Initialize libini
/// @return -1 if unsuccessful
extern int kolibri_libini_init(void);

/// @brief Enumerate sections, calling callback function for each of them.
/// @param filename ini filename
/// @param callback function address: func(f_name, sec_name), where f_name - filename and sec_name - section name found
/// @return -1 if error
extern uint32_t (*LIBINI_enum_sections)(const char* filename, void* callback) __attribute__((__stdcall__));


/// @brief Enumerate keys within a section, calling callback function for each of them.
/// @param filename ini filename
/// @param sec_name section name
/// @param callback callback function address: func(f_name, sec_name, key_name, key_value), where f_name - filename and sec_name - section name found, key_name - key name found, key_name - value of key found
/// @return -1 if error
extern uint32_t (*LIBINI_enum_keys)(const char* filename, const char* sec_name, void* callback) __attribute__((__stdcall__));

/// @brief Read string
/// @param filename ini filename
/// @param sec_name section name
/// @param key_name key name
/// @param buffer destination buffer address
/// @param bufflen buffer size (maximum bytes to read)
/// @param def_val default value to return if no key, section or file found
/// @return -1 if error
extern uint32_t (*LIBINI_get_str)(const char *filename, const char *sec_name, const char *key_name, char *buffer, uint32_t bufflen, uint32_t def_val) __attribute__((__stdcall__));

/// @brief Write string
/// @param filename ini filename
/// @param sec_name section name
/// @param key_name key name
/// @param buffer destination buffer address
/// @param bufflen buffer size (bytes to write)
/// @return -1 if error
extern uint32_t (*LIBINI_set_str)(const char* filename, const char* sec_name, const char* key_name, const char* buffer, uint32_t bufflen) __attribute__((__stdcall__));

/// @brief Read integer
/// @param filename ini filename
/// @param sec_name section name
/// @param key_name key name
/// @param def_val efault value to return if no key, section or file found
/// @return integer, def_val if error
extern uint32_t (*LIBINI_get_int)(const char* filename, const char* sec_name, const char* key_name, uint32_t def_val) __attribute__((__stdcall__));

/// @brief Write integer
/// @param filename ini filename
/// @param sec_name section name
/// @param key_name key name
/// @param value value
/// @return -1 if error
extern uint32_t (*LIBINI_set_int)(const char* filename, const char* sec_name, const char* key_name, uint32_t value) __attribute__((__stdcall__));

/// @brief Read color
/// @param filename ini filename
/// @param sec_name section name
/// @param key_name key name
/// @param def_val default value to return if no key, section or file found
/// @return def value if error, else color
extern uint32_t (*LIBINI_get_color)(const char* filename, const char* sec_name, const char* key_name, uint32_t def_val) __attribute__((__stdcall__));

/// @brief Write color
/// @param filename ini filename
/// @param sec_name section name
/// @param key_name key name
/// @param value value
/// @return -1 if error
extern uint32_t (*LIBINI_set_color)(const char* filename, const char* sec_name, const char* key_name, uint32_t value) __attribute__((__stdcall__));

/// @brief Read shortcut key
/// @param filename ini filename
/// @param sec_name section name
/// @param key_name key name
/// @param def_val default value to return if no key, section or file found
/// @param modifiers pointer to dword variable which receives modifiers state as in 66.4
/// @return def_val if error, else shortcut key value as scancode
/// @result modifiers unchanged (error) / modifiers state for this shortcut
extern uint32_t (*LIBINI_get_shortcut)(const char* filename, const char* sec_name, const char* key_name, uint32_t def_val, const char* modifiers) __attribute__((__stdcall__));

/**
 * \brief buf2d lib
 * <a href="http://wiki.kolibrios.org/wiki/Libs-dev/libini">info on wiki</a>
 * <a href="http://wiki.kolibrios.org/wiki/Libs-dev/libini/ru">info on wiki(ru)</a>
 */

#endif /* KOLIBRI_LIBINI_H */
