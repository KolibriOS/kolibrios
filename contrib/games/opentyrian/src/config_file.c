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
 * \file config_file.c
 * \author Carl Reinke
 * \date 2015
 * \copyright GNU General Public License v2+ or Mozilla Public License 2.0
 */
#include "config_file.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* potential size of decimal representation of type */
#define udecsizeof(t) ((CHAR_BIT * sizeof(t) / 3) + 1)
#define sdecsizeof(t) (udecsizeof(t) + 1)

extern void config_oom( void );

void config_oom( void )
{
	fprintf(stderr, "out of memory\n");
	exit(EXIT_FAILURE);
}

/* string manipulators */

static ConfigString string_init_len( const char *s, size_t n )
{
	ConfigString string;
	
	if (s == NULL)
	{
		CONFIG_STRING_LONG_TAG(string) = true;
		
		string.long_buf = NULL;
	}
	else
	{
		char is_long = n >= COUNTOF(string.short_buf);
		
		CONFIG_STRING_LONG_TAG(string) = is_long;
		
		char *buffer = is_long ?
			string.long_buf = malloc((n + 1) * sizeof(char)) :
			string.short_buf;
		if (buffer == NULL)
			config_oom();
		
		memcpy(buffer, s, n * sizeof(char));
		buffer[n] = '\0';
	}
	
	return string;
}

static void string_deinit( ConfigString *string )
{
	char is_long = CONFIG_STRING_LONG_TAG(*string);
	
	if (is_long)
	{
		free(string->long_buf);
		string->long_buf = NULL;
	}
}

static bool string_equal_len( ConfigString *string, const char *s, size_t n )
{
	const char *cstr = config_string_to_cstr(string);
	return strncmp(cstr, s, n) == 0 && cstr[n] == '\0';
}

/* config manipulators */

static void deinit_section( ConfigSection *section );
static void deinit_option( ConfigOption *option );

void config_init( Config *config )
{
	assert(config != NULL);
	
	config->sections_count = 0;
	config->sections = NULL;
}

void config_deinit( Config *config )
{
	assert(config != NULL);
	
	for (unsigned int s = 0; s < config->sections_count; ++s)
	{
		ConfigSection *section = &config->sections[s];
		
		deinit_section(section);
	}
	
	free(config->sections);
	config->sections = NULL;
}

/* config section manipulators -- internal */

static void init_section( ConfigSection *section, const char *type, size_t type_len, const char *name, size_t name_len )
{
	section->type = string_init_len(type, type_len);
	section->name = string_init_len(name, name_len);
	section->options_count = 0;
	section->options = NULL;
}

static void deinit_section( ConfigSection *section )
{
	for (unsigned int o = 0; o < section->options_count; ++o)
	{
		ConfigOption *option = &section->options[o];
		
		deinit_option(option);
	}
	
	string_deinit(&section->type);
	string_deinit(&section->name);
	
	free(section->options);
	section->options = NULL;
}

/* config section accessors/manipulators -- by type, name */

ConfigSection *config_add_section_len( Config *config, const char *type, size_t type_len, const char *name, size_t name_len )
{
	assert(config != NULL);
	assert(type != NULL);
	
	ConfigSection *sections = realloc(config->sections, (config->sections_count + 1) * sizeof(ConfigSection));
	if (sections == NULL)
		return NULL;
	
	ConfigSection *section = &sections[config->sections_count];
	
	config->sections_count += 1;
	config->sections = sections;
	
	init_section(section, type, type_len, name, name_len);
	
	return section;
}

ConfigSection *config_find_sections( Config *config, const char *type, ConfigSection **save )
{
	assert(config != NULL);
	assert(type != NULL);
	
	ConfigSection *sections_end = &config->sections[config->sections_count];
	
	ConfigSection *section = save != NULL && *save != NULL ?
		*save :
		&config->sections[0];
	
	for (; section < sections_end; ++section)
		if (strcmp(config_string_to_cstr(&section->type), type) == 0)
			break;
	
	if (save != NULL)
		*save = section;
	
	return section < sections_end ? section : NULL;
}

ConfigSection *config_find_section( Config *config, const char *type, const char *name )
{
	assert(config != NULL);
	assert(type != NULL);
	
	ConfigSection *sections_end = &config->sections[config->sections_count];
	
	for (ConfigSection *section = &config->sections[0]; section < sections_end; ++section)
	{
		if (strcmp(config_string_to_cstr(&section->type), type) == 0)
		{
			const char *section_name = config_string_to_cstr(&section->name);
			if ((section_name == NULL || name == NULL) ? section_name == name : strcmp(config_string_to_cstr(&section->name), name) == 0)
				return section;
		}
	}
	
	return NULL;
}

ConfigSection *config_find_or_add_section( Config *config, const char *type, const char *name )
{
	assert(config != NULL);
	assert(type != NULL);
	
	ConfigSection *section = config_find_section(config, type, name);
	
	if (section != NULL)
		return section;
	
	return config_add_section(config, type, name);
}

/* config option manipulators -- internal */

static void init_option_value( ConfigOption *option, const char *value, size_t value_len )
{
	option->values_count = 0;
	option->v.value = string_init_len(value, value_len);
}

static void deinit_option_value( ConfigOption *option )
{
	if (option->values_count != 0)
	{
		ConfigString *values_end = &option->v.values[option->values_count];
		for (ConfigString *value = &option->v.values[0]; value < values_end; ++value)
			string_deinit(value);
		
		free(option->v.values);
		option->v.values = NULL;
	}
	else
	{
		string_deinit(&option->v.value);
	}
}

static void init_option( ConfigOption *option, const char *key, size_t key_len, const char *value, size_t value_len )
{
	option->key = string_init_len(key, key_len);
	init_option_value(option, value, value_len);
}

static void deinit_option( ConfigOption *option )
{
	string_deinit(&option->key);
	deinit_option_value(option);
}

static ConfigOption *append_option( ConfigSection *section, const char *key, size_t key_len, const char *value, size_t value_len )
{
	ConfigOption *options = realloc(section->options, (section->options_count + 1) * sizeof(ConfigSection));
	if (options == NULL)
		return NULL;
	
	ConfigOption *option = &options[section->options_count];
	
	section->options_count += 1;
	section->options = options;
	
	init_option(option, key, key_len, value, value_len);
	
	return option;
}

static ConfigOption *get_option_len( ConfigSection *section, const char *key, size_t key_len )
{
	assert(section != NULL);
	assert(key != NULL);
	
	ConfigOption *options_end = &section->options[section->options_count];
	for (ConfigOption *option = &section->options[0]; option < options_end; ++option)
		if (string_equal_len(&option->key, key, key_len))
			return option;
	
	return NULL;
}

/* config option accessors/manipulators -- by key */

ConfigOption *config_set_option_len( ConfigSection *section, const char *key, size_t key_len, const char *value, size_t value_len )
{
	assert(section != NULL);
	assert(key != NULL);
	
	ConfigOption *option = get_option_len(section, key, key_len);
	
	if (option != NULL)
		return config_set_value_len(option, value, value_len);
	
	return append_option(section, key, key_len, value, value_len);
}

ConfigOption *config_get_option( const ConfigSection *section, const char *key )
{
	assert(section != NULL);
	assert(key != NULL);
	
	ConfigOption *options_end = &section->options[section->options_count];
	for (ConfigOption *option = &section->options[0]; option < options_end; ++option)
		if (strcmp(config_string_to_cstr(&option->key), key) == 0)
			return option;
	
	return NULL;
}

ConfigOption *config_get_or_set_option_len( ConfigSection *section, const char *key, size_t key_len, const char *value, size_t value_len )
{
	assert(section != NULL);
	assert(key != NULL);
	
	ConfigOption *option = get_option_len(section, key, key_len);
	
	if (option != NULL)
		return option;
	
	return append_option(section, key, key_len, value, value_len);
}

void config_set_string_option_len( ConfigSection *section, const char *key, size_t key_len, const char *value, size_t value_len )
{
	if (config_set_option_len(section, key, key_len, value, value_len) == NULL)
		config_oom();
}

bool config_get_string_option( const ConfigSection *section, const char *key, const char **out_value )
{
	assert(section != NULL);
	assert(key != NULL);
	
	ConfigOption *option = config_get_option(section, key);
	if (option != NULL)
	{
		const char *value = config_get_value(option);
		if (value != NULL)
		{
			*out_value = value;
			return true;
		}
	}
	
	return false;
}

const char *config_get_or_set_string_option( ConfigSection *section, const char *key, const char *value )
{
	if (!config_get_string_option(section, key, &value))
		config_set_string_option_len(section, key, strlen(key), value, value == NULL ? 0 : strlen(value));
	return value;
}

static const char *bool_values[][2] = 
{
	{ "0", "1" },
	{ "no", "yes" },
	{ "off", "on" },
	{ "false", "true" },
};

void config_set_bool_option( ConfigSection *section, const char *key, bool value, ConfigBoolStyle style )
{
	if (config_set_option(section, key, bool_values[style][value ? 1 : 0]) == NULL)
		config_oom();
}

bool config_get_bool_option( const ConfigSection *section, const char *key, bool *out_value )
{
	assert(section != NULL);
	assert(key != NULL);
	assert(out_value != NULL);
	
	const char *value;
	if (config_get_string_option(section, key, &value))
	{
		for (size_t i = 0; i < COUNTOF(bool_values); ++i)
		{
			for (size_t j = 0; j < COUNTOF(bool_values[i]); ++j)
			{
				if (strcmp(value, bool_values[i][j]) == 0)
				{
					*out_value = j == 0 ? false : true;
					return true;
				}
			}
		}
	}
	
	return false;
}

bool config_get_or_set_bool_option( ConfigSection *section, const char *key, bool value, ConfigBoolStyle style )
{
	if (!config_get_bool_option(section, key, &value))
		config_set_bool_option(section, key, value, style);
	return value;
}

void config_set_int_option( ConfigSection *section, const char *key, int value )
{
	assert(key != NULL);
	
	char buffer[sdecsizeof(int) + 1];
	int buffer_len = snprintf(buffer, sizeof(buffer), "%i", value);
	
	if (config_set_option_len(section, key, strlen(key), buffer, buffer_len) == NULL)
		config_oom();
}

bool config_get_int_option( const ConfigSection *section, const char *key, int *out_value )
{
	assert(section != NULL);
	assert(key != NULL);
	assert(out_value != NULL);
	
	const char *value;
	if (config_get_string_option(section, key, &value))
	{
		int i;
		int n;
		if (sscanf(value, "%i%n", &i, &n) > 0 && value[n] == '\0')  /* must be entire string */
		{
			*out_value = i;
			return true;
		}
	}
	
	return false;
}

int config_get_or_set_int_option( ConfigSection *section, const char *key, int value )
{
	if (!config_get_int_option(section, key, &value))
		config_set_int_option(section, key, value);
	return value;
}

void config_set_uint_option( ConfigSection *section, const char *key, unsigned int value )
{
	assert(key != NULL);
	
	char buffer[udecsizeof(unsigned int) + 1];
	int buffer_len = snprintf(buffer, sizeof(buffer), "%u", value);
	
	if (config_set_option_len(section, key, strlen(key), buffer, buffer_len) == NULL)
		config_oom();
}

bool config_get_uint_option( const ConfigSection *section, const char *key, unsigned int *out_value )
{
	assert(section != NULL);
	assert(key != NULL);
	assert(out_value != NULL);
	
	const char *value;
	if (config_get_string_option(section, key, &value))
	{
		unsigned int u;
		int n;
		if (sscanf(value, "%u%n", &u, &n) > 0 && value[n] == '\0')  /* must be entire string */
		{
			*out_value = u;
			return true;
		}
	}
	
	return false;
}

unsigned int config_get_or_set_uint_option( ConfigSection *section, const char *key, unsigned int value )
{
	if (!config_get_uint_option(section, key, &value))
		config_set_uint_option(section, key, value);
	return value;
}

/* config option accessors/manipulators -- by reference */

ConfigOption *config_set_value_len( ConfigOption *option, const char *value, size_t value_len )
{
	assert(option != NULL);
	
	deinit_option_value(option);
	
	init_option_value(option, value, value_len);
	
	return option;
}

ConfigOption *config_add_value_len( ConfigOption *option, const char *value, size_t value_len )
{
	assert(option != NULL);
	assert(value != NULL);
	
	/* convert 'item' to 'list' */
	if (option->values_count == 0 && config_string_to_cstr(&option->v.value) != NULL)
	{
		ConfigString option_value = option->v.value;
		
		ConfigString *values = malloc(2 * sizeof(ConfigString));
		if (values == NULL)
			return NULL;
		
		option->v.values = values;
		option->v.values[0] = option_value;
		option->v.values[1] = string_init_len(value, value_len);
		option->values_count = 2;
	}
	else
	{
		ConfigString *values = realloc(option->v.values, (option->values_count + 1) * sizeof(ConfigString));
		if (values == NULL)
			return NULL;
		
		option->v.values = values;
		option->v.values[option->values_count] = string_init_len(value, value_len);
		option->values_count += 1;
	}
	
	return option;
}

ConfigOption *config_remove_value( ConfigOption *option, unsigned int i )
{
	assert(option != NULL);
	
	if (!config_is_value_list(option))
	{
		if (i > 0)
			return NULL;
		
		config_set_value_len(option, NULL, 0);
	}
	else
	{
		if (i >= option->values_count)
			return NULL;
		
		string_deinit(&option->v.values[i]);
		memmove(&option->v.values[i], &option->v.values[i + 1], (option->values_count - i - 1) * sizeof(ConfigString));
		
		if (option->values_count - 1 == 0)
		{
			option->v.value = string_init_len(NULL, 0);
			option->values_count = 0;
		}
		else
		{
			ConfigString *values = realloc(option->v.values, (option->values_count - 1) * sizeof(ConfigString));
			if (values == NULL)
				return NULL;
			
			option->v.values = values;
			option->values_count -= 1;
		}
	}
	
	return option;
}

const char *config_get_value( const ConfigOption *option )
{
	if (option == NULL || option->values_count != 0)
		return NULL;
	
	return config_string_to_cstr(&option->v.value);
}

/* config parser */

static bool is_whitespace( char c )
{
	return c == '\t' || c == ' ';
}

static bool is_end( char c )
{
	return c == '\0' || c == '\n' || c == '\r';
}

static bool is_whitespace_or_end( char c )
{
	return is_whitespace(c) || is_end(c);
}

typedef enum
{
	INVALID_DIRECTIVE = 0,
	SECTION_DIRECTIVE,
	ITEM_DIRECTIVE,
	LIST_DIRECTIVE,
} Directive;

static Directive match_directive( const char *buffer, size_t *index )
{
	size_t i = *index;
	
	while (is_whitespace(buffer[i]))
		++i;
	
	Directive directive;
	
	if (strncmp("section", &buffer[i], 7) == 0)
	{
		directive = SECTION_DIRECTIVE;
		i += 7;
	}
	else if (strncmp("item", &buffer[i], 4) == 0)
	{
		directive = ITEM_DIRECTIVE;
		i += 4;
	}
	else if (strncmp("list", &buffer[i], 4) == 0)
	{
		directive = LIST_DIRECTIVE;
		i += 4;
	}
	else
	{
		return INVALID_DIRECTIVE;
	}
	
	if (!is_whitespace_or_end(buffer[i]))
		return INVALID_DIRECTIVE;
	
	*index = i;
	
	return directive;
}

static bool match_nonquote_field( const char *buffer, size_t *index, size_t *length )
{
	size_t i = *index;
	
	for (; ; ++i)
	{
		char c = buffer[i];
		
		if (is_whitespace_or_end(c))
		{
			break;
		}
		else if (c <= ' ' || c > '~' || c == '#' || c == '\'' || c == '"')
		{
			return false;
		}
	}
	
	*length = i - *index;
	*index = i;
	
	return *length > 0;
}

static bool parse_quote_field( char *buffer, size_t *index, size_t *length )
{
	size_t i = *index;
	size_t o = *index;
	
	char quote = buffer[i];
	
	for (; ; )
	{
		char c = buffer[++i];
		
		if (c == quote)
		{
			++i;
			break;
		}
		else if (c == '\\')
		{
			c = buffer[++i];
			if (c == quote)
			{
				buffer[o++] = quote;
			}
			else
			{
				switch (c)
				{
				case 't':
					buffer[o++] = '\t';
					break;
				case 'n':
					buffer[o++] = '\n';
					break;
				case 'r':
					buffer[o++] = '\r';
					break;
				case '\\':
					buffer[o++] = '\\';
					break;
				case 'x':
					/* parse two hex digits */
					c = buffer[++i];
					char m = (c >= '0' && c <= '9') ? '0' :
					         (c >= 'a' && c <= 'f') ? 'a' - 10 :
					         (c >= 'A' && c <= 'F') ? 'A' - 10 : 0;
					if (m == 0)
						return false;
					char h = c - m;
					c = buffer[++i];
					m = (c >= '0' && c <= '9') ? '0' :
					    (c >= 'a' && c <= 'f') ? 'a' - 10 :
					    (c >= 'A' && c <= 'F') ? 'A' - 10 : 0;
					if (m == 0)
						return false;
					buffer[o++] = (h << 4) | (c - m);
					break;
				default:
					return false;
				}
			}
		}
		else if (c >= ' ' && c <= '~')
		{
			buffer[o++] = c;
		}
		else
		{
			return false;
		}
	}
	
	*length = o - *index;
	*index = i;
	
	return true;
}

static bool parse_field( char *buffer, size_t *index, size_t *start, size_t *length )
{
	size_t i = *index;
	
	while (is_whitespace(buffer[i]))
		++i;
	
	*start = i;
	
	if (buffer[i] == '"' || buffer[i] == '\'')
	{
		if (!parse_quote_field(buffer, &i, length))
			return false;
	}
	else
	{
		if (!match_nonquote_field(buffer, &i, length))
			return false;
	}
	
	if (!is_whitespace_or_end(buffer[i]))
		return INVALID_DIRECTIVE;
	
	*index = i;
	
	return true;
}

bool config_parse( Config *config, FILE *file )
{
	assert(config != NULL);
	assert(file != NULL);
	
	config_init(config);
	
	ConfigSection *section = NULL;
	ConfigOption *option = NULL;
	
	size_t buffer_cap = 128;
	char *buffer = malloc(buffer_cap * sizeof(char));
	if (buffer == NULL)
		config_oom();
	size_t buffer_end = 1;
	buffer[buffer_end - 1] = '\0';
	
	for (size_t line = 0, next_line = 0; ; line = next_line)
	{
		/* find begining of next line */
		while (next_line < buffer_end)
		{
			char c = buffer[next_line];
			
			if (c == '\0' && next_line == buffer_end - 1)
			{
				if (line > 0)
				{
					/* shift to front */
					memmove(&buffer[0], &buffer[line], buffer_end - line);
					buffer_end -= line;
					next_line -= line;
					line = 0;
				}
				else if (buffer_end > 1)
				{
					/* need larger capacity */
					buffer_cap *= 2;
					char *new_buffer = realloc(buffer, buffer_cap * sizeof(char));
					if (new_buffer == NULL)
						config_oom();
					buffer = new_buffer;
				}
				
				size_t read = fread(&buffer[buffer_end - 1], sizeof(char), buffer_cap - buffer_end, file);
				if (read == 0)
					break;
				
				buffer_end += read;
				buffer[buffer_end - 1] = '\0';
			}
			else
			{
				++next_line;
				
				if (c == '\n' || c == '\r')
					break;
			}
		}
		
		/* if at end of file */
		if (next_line == line)
			break;
		
		size_t i = line;
		
		Directive directive = match_directive(buffer, &i);
		
		switch (directive)
		{
		case INVALID_DIRECTIVE:
			continue;
		case SECTION_DIRECTIVE:
			{
				size_t type_start;
				size_t type_length;
				
				if (!parse_field(buffer, &i, &type_start, &type_length))
					continue;
				
				size_t name_start;
				size_t name_length;
				
				bool has_name = parse_field(buffer, &i, &name_start, &name_length);
				
				section = config_add_section_len(config,
						&buffer[type_start], type_length,
						has_name ? &buffer[name_start] : NULL, has_name ? name_length : 0);
				if (section == NULL)
					config_oom();
				option = NULL;
			}
			break;
		case ITEM_DIRECTIVE:
		case LIST_DIRECTIVE:
			{
				if (section == NULL)
					continue;
				
				size_t key_start;
				size_t key_length;
				
				if (!parse_field(buffer, &i, &key_start, &key_length))
					continue;
				
				size_t value_start;
				size_t value_length;
				
				if (!parse_field(buffer, &i, &value_start, &value_length))
					continue;
				
				if (directive == ITEM_DIRECTIVE)
				{
					option = config_set_option_len(section,
							&buffer[key_start], key_length,
							&buffer[value_start], value_length);
				}
				else
				{
					if (option == NULL || !string_equal_len(&option->key, &buffer[key_start], key_length))
						option = config_get_or_set_option_len(section,
								&buffer[key_start], key_length,
								NULL, 0);
					if (option != NULL)
						option = config_add_value_len(option,
								&buffer[value_start], value_length);
				}
				if (option == NULL)
					config_oom();
			}
			break;
		}
		
		assert(i <= next_line);
	}
	
	free(buffer);
	
	return config;
}

/* config writer */

static void write_field( const ConfigString *field, FILE *file )
{
	fputc('\'', file);
	
	char buffer[128];
	size_t o = 0;
	
	for (const char *ci = config_string_to_cstr(field); *ci != '\0'; ++ci)
	{
		char c = *ci;
		
		size_t l;
		switch (c)
		{
			case '\t':
			case '\n':
			case '\r':
			case '\'':
			case '\\':
				l = 2;
				break;
			default:
				l = (c >= ' ' && c <= '~') ? 1 : 4;
				break;
		}
		
		if (o + l > COUNTOF(buffer))
		{
			fwrite(buffer, sizeof(*buffer), o, file);
			o = 0;
		}
		
		switch (l)
		{
			case 1:
				buffer[o++] = c;
				break;
			case 2:
				switch (c)
				{
					case '\t':
						buffer[o++] = '\\';
						buffer[o++] = 't';
						break;
					case '\n':
						buffer[o++] = '\\';
						buffer[o++] = 'n';
						break;
					case '\r':
						buffer[o++] = '\\';
						buffer[o++] = 'r';
						break;
					case '\'':
					case '\\':
						buffer[o++] = '\\';
						buffer[o++] = c;
						break;
				}
				break;
			case 4:
				buffer[o++] = '\\';
				buffer[o++] = 'x';
				char n = (c >> 4) & 0x0f;
				buffer[o++] = (n < 10 ? '0' : ('a' - 10)) + n;
				n = c & 0x0f;
				buffer[o++] = (n < 10 ? '0' : ('a' - 10)) + n;
				break;
		}
	}
	
	if (o > 0)
		fwrite(buffer, sizeof(*buffer), o, file);
	
	fputc('\'', file);
}

void config_write( const Config *config, FILE *file )
{
	assert(config != NULL);
	assert(file != NULL);
	
	for (unsigned int s = 0; s < config->sections_count; ++s)
	{
		ConfigSection *section = &config->sections[s];
		
		fputs("section ", file);
		write_field(&section->type, file);
		if (config_string_to_cstr(&section->name) != NULL)
		{
			fputc(' ', file);
			write_field(&section->name, file);
		}
		fputc('\n', file);
		
		for (unsigned int o = 0; o < section->options_count; ++o)
		{
			ConfigOption *option = &section->options[o];
			
			if (option->values_count == 0 && config_string_to_cstr(&option->v.value) != NULL)
			{
				fputs("\titem ", file);
				write_field(&option->key, file);
				fputc(' ', file);
				write_field(&option->v.value, file);
				fputc('\n', file);
			}
			else
			{
				ConfigString *values_end = &option->v.values[option->values_count];
				for (ConfigString *value = &option->v.values[0]; value < values_end; ++value)
				{
					fputs("\tlist ", file);
					write_field(&option->key, file);
					fputc(' ', file);
					write_field(value, file);
					fputc('\n', file);
				}
			}
		}
		
		fputc('\n', file);
	}
}
