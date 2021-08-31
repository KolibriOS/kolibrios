/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2015  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
/*!
 * \file config_file.h
 * \author Carl Reinke
 * \date 2015
 * \copyright GNU General Public License v2+ or Mozilla Public License 2.0
 */
#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef COMPILE_TIME_ASSERT
/*!
 * \brief Cause compile error if compile-time computable condition fails.
 * 
 * \param[in] name the unique identifier of the assertion
 * \param[in] cond the condition
 */
#define COMPILE_TIME_ASSERT(name, cond) typedef int assert_ ## name[(cond) * 2 - 1]
#endif

#ifndef COUNTOF
/*!
 * \brief Calculate the number of elements in a fixed-length array.
 * 
 * \param[in] a the fixed-length array
 * \return the number of elements in the array
 */
#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))
#endif

/* string type */

/*!
 * \brief A short-string-optimizing string type.
 * 
 * This struct allows for storing up to 15 characters (plus a terminating \c '\0') inline.  For
 * longer strings memory will be allocated.
 * 
 * The tag for this union is the last character of \p short_buf:
 * \li if \c '\0' then \p short_buf is valid,
 * \li otherwise \p long_buf is valid.
 */
typedef union
{
	/*!
	 * \brief The inline buffer for short strings.
	 */
	char short_buf[16];
	
	/*!
	 * \brief The buffer for long strings.
	 *
	 * May be \c NULL.
	 */
	char *long_buf;
} ConfigString;

/*! \cond suppress_doxygen */
COMPILE_TIME_ASSERT(string_short_buf_sufficient, sizeof(char *) + 1 <= COUNTOF(((ConfigString *)NULL)->short_buf));
/*! \endcond */

/*! \cond suppress_doxygen */
#define CONFIG_STRING_LONG_TAG(s) ((s).short_buf[COUNTOF((s).short_buf) - 1])
/*! \endcond */

/*!
 * \brief Return a C-string backed by a string.
 * 
 * \param[in] string the string
 * \return the C-string
 */
static inline const char *config_string_to_cstr( const ConfigString *string )
{
	assert(string != NULL);
	char is_long = CONFIG_STRING_LONG_TAG(*string);
	return is_long ?
		string->long_buf :
		string->short_buf;
}

/* config types */

/*!
 * \brief An option consisting of one (an item) or many (a list) values.
 */
typedef struct
{
	/*!
	 * \brief The key of the option.
	 */
	ConfigString key;
	
	/*!
	 * \brief The number of values in the option if it is a 'list' option.
	 *
	 * If \c 0 then the option \e may be an 'item' option.
	 * 
	 * \see ::ConfigOption::value
	 */
	unsigned int values_count;
	
	/*!
	 * \brief The value or values.
	 * 
	 * The tag for this union is \p value_count:
	 * \li if \c 0 then \p value is valid,
	 * \li otherwise \p values is valid.
	 */
	union
	{
		/*!
		 * \brief The value of an 'item' option or an empty 'list' option.
		 * 
		 * If this field is \c NULL then the option is an empty 'list' option.
		 */
		ConfigString value;
		
		/*!
		 * \brief The values of a non-empty 'list' option.
		 */
		ConfigString *values;
	} v;
} ConfigOption;

/*!
 * \brief A section consisting of options.
 */
typedef struct
{
	/*!
	 * \brief The type of the section.
	 */
	ConfigString type;
	
	/*!
	 * \brief The optional name of the section.
	 *
	 * May be \c NULL.
	 */
	ConfigString name;
	
	/*!
	 * \brief The number of options in the section.
	 */
	unsigned int options_count;
	
	/*!
	 * \brief The options in the section.
	 *
	 * \c NULL if \p options_count is \c 0.
	 */
	ConfigOption *options;
} ConfigSection;

/*!
 * \brief A configuration consisting of sections.
 */
typedef struct
{
	/*!
	 * \brief The number of sections in the configuration.
	 */
	unsigned int sections_count;
	
	/*!
	 * \brief The sections in the configuration.
	 *
	 * \c NULL if \p sections_count is \c 0.
	 */
	ConfigSection *sections;
} Config;

/* config manipulators */

/*!
 * \brief Initialize a configuration.
 * 
 * \param[in] config the configuration
 * \return void
 */
extern void config_init( Config *config );

/*!
 * \brief Release any memory allocated inside a configuration.
 * 
 * \param[in] config the configuration
 * \return void
 */
extern void config_deinit( Config *config );

/*!
 * \brief Parse a configuration from a file.
 * 
 * \param[in] config the uninitalized configuration
 * \param[in] file the file handle
 * \return whether parsing succeeded
 */
extern bool config_parse( Config *config, FILE *file );

/*!
 * \brief Write a configuration to a file.
 * 
 * \param[in] config the configuration
 * \param[in] file the file handle
 * \return void
 */
extern void config_write( const Config *config, FILE *file );

/* config section accessors/manipulators -- by type, name */

/*! \see ::config_add_section() */
extern ConfigSection *config_add_section_len( Config *config, const char *type, size_t type_len, const char *name, size_t name_len );

/*!
 * \brief Add a section to a configuration.
 * 
 * \param[in] config the configuration to contain the section
 * \param[in] type the type of the section
 * \param[in] name the name of the section; may be \c NULL
 * \return the added section; \c NULL if out of memory
 */
static inline ConfigSection *config_add_section( Config *config, const char *type, const char *name)
{
	assert(type != NULL);
	return config_add_section_len(config, type, strlen(type), name, name == NULL ? 0 : strlen(name));
}

// TODO: extern Config *config_remove_section( Config *config, unsigned int i );

/*!
 * \brief Iterate sections by type.
 * 
 * \param[in] config the configuration containing the sections
 * \param[in] type the type of the section
 * \param[in,out] save the saved state of the iterator; initialize \c *save to \c NULL before
 *                     iteration
 * \return the section; \c NULL if iteration finished
 */
extern ConfigSection *config_find_sections( Config *config, const char *type, ConfigSection **save );

/*!
 * \brief Find a section by type and name.
 * 
 * \param[in] config the configuration containing the section
 * \param[in] type the type of the section
 * \param[in] name the name of the section
 * \return the section; \c NULL if it does not exist
 */
extern ConfigSection *config_find_section( Config *config, const char *type, const char *name );

/*!
 * \brief Find a section by type and name, creating the section if it did not exist.
 * 
 * \param[in] config the configuration containing the section
 * \param[in] type the type of the section
 * \param[in] name the name of the section; may be \c NULL
 * \return the section; \c NULL if out of memory
 */
extern ConfigSection *config_find_or_add_section( Config *config, const char *type, const char *name );

/* config option accessors/manipulators -- by key */

/*! \see ::config_set_option() */
extern ConfigOption *config_set_option_len( ConfigSection *section, const char *key, size_t key_len, const char *value, size_t value_len );

/*!
 * \brief Set a value of an 'item' option by key, creating the option if necessary.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the item value; \c NULL to set an emtpy 'list' option instead of an 'item'
 *                  option (can be used to delete an 'item' option)
 * \return the option; \c NULL if out of memory
 */
static inline ConfigOption *config_set_option( ConfigSection *section, const char *key, const char *value)
{
	assert(key != NULL);
	return config_set_option_len(section, key, strlen(key), value, value == NULL ? 0 : strlen(value));
}

/*!
 * \brief Get an option by key.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \return the option; \c NULL if it does not exist
 */
extern ConfigOption *config_get_option( const ConfigSection *section, const char *key );

/*! \see ::config_get_or_set_option() */
extern ConfigOption *config_get_or_set_option_len( ConfigSection *section, const char *key, size_t key_len, const char *value, size_t value_len );

/*!
 * \brief Get an option by key, creating an 'item' option if the option did not exist.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the default item value; \c NULL to set an empty 'list' option instead of an
 *                  'item' option
 * \return the option; \c NULL if out of memory
 */
static inline ConfigOption *config_get_or_set_option( ConfigSection *section, const char *key, const char *value )
{
	assert(key != NULL);
	return config_get_or_set_option_len(section, key, strlen(key), value, value == NULL ? 0 : strlen(value));
}

/*! \see ::config_set_string_option() */
extern void config_set_string_option_len( ConfigSection *section, const char *key, size_t key_len, const char *value, size_t value_len );

/*!
 * \brief Set a string value of an 'item' option by key, creating the option if necessary.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the item value
 * \return void
 */
static inline void config_set_string_option( ConfigSection *section, const char *key, const char *value )
{
	assert(key != NULL);
	config_set_string_option_len(section, key, strlen(key), value, value == NULL ? 0 : strlen(value));
}

/*!
 * \brief Get a string value of an 'item' option by key.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[out] out_value the item value if a valid option exists; otherwise unset
 * \return whether \p out_value was set
 */
extern bool config_get_string_option( const ConfigSection *section, const char *key, const char **out_value );

/*!
 * \brief Get a string value of an 'item' option by key, setting the option if it was invalid or
 *        creating the option if it did not exist.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the default item value
 * \return the value
 */
extern const char *config_get_or_set_string_option( ConfigSection *section, const char *key, const char *value );

/*!
 * \brief The styles of boolean values.
 */
typedef enum
{
	ZERO_ONE = 0,
	NO_YES = 1,
	OFF_ON = 2,
	FALSE_TRUE = 3,
} ConfigBoolStyle;

/*!
 * \brief Set a boolean value of an 'item' option by key, creating the option if necessary.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the item value
 * \param[in] style the style of boolean value
 * \return void
 */
extern void config_set_bool_option( ConfigSection *section, const char *key, bool value, ConfigBoolStyle style );

/*!
 * \brief Get a boolean value of an 'item' option by key.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[out] out_value the item value if a valid option exists; otherwise unset
 * \return whether \p out_value was set
 */
extern bool config_get_bool_option( const ConfigSection *section, const char *key, bool *out_value );

/*!
 * \brief Get a boolean value of an 'item' option by key, setting the option if it was invalid or
 *        creating the option if it did not exist.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the default item value
 * \param[in] style the style of boolean value
 * \return the value
 */
extern bool config_get_or_set_bool_option( ConfigSection *section, const char *key, bool value, ConfigBoolStyle style );

/*!
 * \brief Set an integer value of an 'item' option by key, creating the option if necessary.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the item value
 * \return void
 */
extern void config_set_int_option( ConfigSection *section, const char *key, int value );

/*!
 * \brief Get an integer value of an 'item' option by key.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[out] out_value the item value if a valid option exists; otherwise unset
 * \return whether \p out_value was set
 */
extern bool config_get_int_option( const ConfigSection *section, const char *key, int *out_value );

/*!
 * \brief Get an integer value of an 'item' option by key, setting the option if it was invalid or
 *        creating the option if it did not exist.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the default item value
 * \return the value
 */
extern int config_get_or_set_int_option( ConfigSection *section, const char *key, int value );

/*!
 * \brief Set an unsigned integer value of an 'item' option by key, creating the option if
 *        necessary.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the item value
 * \return void
 */
extern void config_set_uint_option( ConfigSection *section, const char *key, unsigned int value );

/*!
 * \brief Get an unsigned integer value of an 'item' option by key.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[out] out_value the item value if a valid option exists; otherwise unset
 * \return whether \p out_value was set
 */
extern bool config_get_uint_option( const ConfigSection *section, const char *key, unsigned int *out_value );

/*!
 * \brief Get an unsigned integer value of an 'item' option by key, setting the option if it was
 *        invalid or creating the option if it did not exist.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the default item value
 * \return the value
 */
extern unsigned int config_get_or_set_uint_option( ConfigSection *section, const char *key, unsigned int value );

/* config option accessors/manipulators -- by reference */

/*! \see ::config_set_value() */
extern ConfigOption *config_set_value_len( ConfigOption *option, const char *value, size_t value_len );

/*!
 * \brief Set the value of an 'item' option.
 * 
 * \param[in] option the option
 * \param[in] value the value
 * \return the option; \c NULL if out of memory
 */
static inline ConfigOption *config_set_value( ConfigOption *option, const char *value )
{
	return config_set_value_len(option, value, value == NULL ? 0 : strlen(value));
}

/*! \see ::config_add_value() */
extern ConfigOption *config_add_value_len( ConfigOption *option, const char *value, size_t value_len );

/*!
 * \brief Add a value to a 'list' option.
 * 
 * \param[in] option the option
 * \param[in] value the value
 * \return the option; \c NULL if out of memory
 */
static inline ConfigOption *config_add_value( ConfigOption *option, const char *value )
{
	assert(value != NULL);
	return config_add_value_len(option, value, strlen(value));
}

/*!
 * \brief Remove a value from a 'list' option.
 * 
 * \param[in] option the option
 * \param[in] i the index of the value
 * \return the option; \c NULL if out of memory or invalid \p index
 */
extern ConfigOption *config_remove_value( ConfigOption *option, unsigned int i );

/*!
 * \brief Get the value of an 'item' option.
 * 
 * \param[in] option the option
 * \return the value; \c NULL if \p option was \c NULL or was a 'list' option
 */
extern const char *config_get_value( const ConfigOption *option );

/*!
 * \brief Get the value that indicates whether the option is a 'list' option.
 * 
 * \param[in] option the option
 * \return whether the option is a 'list' option
 */
static inline bool config_is_value_list( const ConfigOption *option )
{
	assert(option != NULL);
	return option->values_count > 0 ||
		config_string_to_cstr(&option->v.value) == NULL;
}

/*!
 * \brief Get the number of values assigned to the option.
 * 
 * \param[in] option the option
 * \return \c 1 if the option is an 'item' option; the number of elements if the option is a 'list'
 *         option
 */
static inline unsigned int config_get_value_count( const ConfigOption *option )
{
	assert(option != NULL);
	return option->values_count == 0 ?
		(config_string_to_cstr(&option->v.value) == NULL ? 0 : 1) :
		option->values_count;
}

/*!
 * \brief Iterate over the values assigned to the option.
 * 
 * \param[out] string_value the value variable to declare
 * \param[in] option the option
 */
#define foreach_option_value( string_value, option ) \
	for (ConfigOption *_option = (option); _option != NULL; _option = NULL) \
	for (ConfigString *_values_begin = _option->values_count == 0 ? &_option->v.value : &_option->v.values[0], \
	                  *_values_end = _option->values_count == 0 ? _values_begin + 1 : &_option->v.values[_option->values_count], \
	                  *_value = _values_begin; _value < _values_end; ++_value) \
	for (const char *(string_value) = config_string_to_cstr(_value); (string_value) != NULL; (string_value) = NULL)

/*!
 * \brief Iterate over the values assigned to the option.
 * 
 * \param[out] i the index variable to declare
 * \param[out] string_value the value variable to declare
 * \param[in] option the option
 */
#define foreach_option_i_value( i, string_value, option ) \
	for (unsigned int (i) = 0; (i) == 0; (i) = ~0) \
	for (ConfigOption *_option = (option); _option != NULL; _option = NULL) \
	for (ConfigString *_values_begin = _option->values_count == 0 ? &_option->v.value : &_option->v.values[0], \
	                  *_values_end = _option->values_count == 0 ? _values_begin + 1 : &_option->v.values[_option->values_count], \
	                  *_value = _values_begin; _value < _values_end; ++_value, (i) = _value - _values_begin) \
	for (const char *(string_value) = config_string_to_cstr(_value); (string_value) != NULL; (string_value) = NULL)

/*!
 * \brief Remove a value from an option during iteration.  Should be followed by \c continue.
 */
#define foreach_remove_option_value() \
	{ \
		extern void config_oom( void ); \
		unsigned int _value_i = _value - _values_begin; \
		if (config_remove_value(_option, _value_i) == NULL) \
			config_oom(); \
		_values_begin = _option->values_count == 0 ? &_option->v.value : &_option->v.values[0]; \
		_values_end = _option->values_count == 0 ? _values_begin + 1 : &_option->v.values[_option->values_count]; \
		_value = _values_begin + _value_i - 1; \
	}

#endif
