import re
import os
import argparse

# Parameters
# Path to doxygen folder to make doxygen files in: -o <path>
doxygen_src_path = 'docs/doxygen'
# Remove generated doxygen files: --clean
clean_generated_stuff = False
# Dump all defined symbols: --dump
dump_symbols = False
# Print symbol stats: --stats
print_stats = False
# Do not write warnings file: --nowarn
enable_warnings = True

# Constants
link_root = "http://websvn.kolibrios.org/filedetails.php?repname=Kolibri+OS&path=/kernel/trunk"

# Warning list
warnings = ""

# Parse arguments
parser = argparse.ArgumentParser()
parser.add_argument("-o", help="Doxygen output folder")
parser.add_argument("--clean", help="Remove generated files", action="store_true")
parser.add_argument("--dump", help="Dump all defined symbols", action="store_true")
parser.add_argument("--stats", help="Print symbol stats", action="store_true")
parser.add_argument("--nowarn", help="Do not write warnings file", action="store_true")
args = parser.parse_args()
doxygen_src_path = args.o if args.o else 'docs/doxygen'
clean_generated_stuff = args.clean
dump_symbols = args.dump
print_stats = args.stats
enable_warnings = not args.nowarn

# kernel_structure["filename"] = {
#   [ [],    # [0] Variables - [ line, name ]
#     [],    # [1] Macros - [ line, name ]
#     [],    # [2] Procedures - [ line, name ]
#     [],    # [3] Labels - [ line, name ]
#     [] ] } # [4] Structures - [ line, name ]
VARIABLES = 0
MACROS = 1
PROCEDURES = 2
LABELS = 3
STRUCTURES = 4
kernel_structure = {}

class AsmVariable:
	def __init__(self, line, name, type, init, comment, line_span):
		self.line = line
		self.name = name
		self.type = type
		self.init = init
		self.comment = comment # Comment after the definition (a dd 0 ; Comment)
		self.line_span = line_span # How much .asm lines its definition takes

class AsmFunction:
	def __init__(self, line, name):
		self.line = line
		self.name = name

class AsmLabel:
	def __init__(self, line, name):
		self.line = line
		self.name = name

class AsmMacro:
	def __init__(self, asm_file_name, line, name, comment, args):
		self.file = asm_file_name
		self.line = line
		self.name = name
		self.comment = comment
		self.args = args

class AsmStruct:
	def __init__(self, line, name):
		self.line = line
		self.name = name

def parse_variable(asm_file_name, lines, line_idx):
	global warnings

	def curr():
		try: return line[i]
		except: return ''

	# Returns current and then increments current index
	def step():
		nonlocal i
		c = curr()
		i += 1
		return c

	line = lines[line_idx]
	i = 0
	# Skip first spaces
	while curr().isspace(): step()
	# Get name
	name = ""
	while curr().isalnum() or curr() == '_' or curr() == '.': name += step()
	# Skip spaces after variable name
	while curr().isspace(): step()
	# Get type specifier (db, dd, etc.)
	type = ""
	while curr().isalnum() or curr() == '_': type += step()
	# Skip spaces after type specifier
	while curr().isspace(): step()
	# Get initial value (everything up to end of the line or comment)
	init = ""
	while curr() and curr() != ';': init += step()
	# Get comment
	comment = ""
	if curr() == ';':
		step() # Skip ';'
		while curr(): comment += step()
	# Process type
	if type == "db": type = "byte"
	elif type == "dw": type = "word"
	elif type == "dd": type = "dword"
	elif type == "dq": type = "qword"
	else: raise Exception(f"Unexpected type: '{type}' (i = {i})")
	# Process comment
	if comment == "": comment = "Undocumented"
	else:
		comment = comment.lstrip()
		if (len(comment) == 0):
			comment = "!!! EMPTY_COMMENT"
			warnings += f"{asm_file_name}:{line_idx + 1}: Empty comment in\n"
		if comment[0].islower():
			warnings += f"{asm_file_name}:{line_idx + 1}: Сomment sarting with lowercase\n"
	# Build the result
	result = AsmVariable(line_idx + 1, name, type, init, comment, 1)
	return (1, result)

def is_id(c):
	return c.isprintable() and c not in "+-/*=<>()[]{}:,|&~#`'\" \n\r\t\v"

def get_comment_begin(line):
	result = len(line)
	in_str = False
	for i in range(len(line)):
		if in_str:
			if line[i] == in_str: in_str = False
			i += 1
		elif line[i] == '\'' or line[i] == '\"':
			in_str = line[i]
			i += 1
		elif line[i] == ';':
			result = i
			break
		else:
			i += 1
	return result

def get_comment(line):
	return line[get_comment_begin(line):]

def remove_comment(line):
	return line[0:get_comment_begin(line)]

def insert_comment(line, comment):
	comment_begin = get_comment_begin(line)
	line_left = line[:get_comment_begin(line)]
	line_right = line[get_comment_begin(line):]
	return line_left + comment + line_right

def has_line_wrap(line):
	if remove_comment(line).rstrip()[-1] == '\\':
		return True
	return False

def remove_line_wrap(line):
	if remove_comment(line).rstrip()[-1] == '\\':
		return remove_comment(line).rstrip()[:-1]
	return line

def parse_macro(asm_file_name, lines, line_idx):
	line_idx_orig = line_idx
	global warnings

	def curr():
		try: return line[i]
		except: return ''

	# Returns current and then increments current index
	def step():
		nonlocal i
		c = curr()
		i += 1
		return c

	line = lines[line_idx]
	# Handle line wraps ('\' at the end)
	while has_line_wrap(line):
		next_line = lines[line_idx + 1]
		prev_line_comment = get_comment(line)
		line = remove_line_wrap(line) + insert_comment(next_line, prev_line_comment)
		line_idx += 1

	i = 0
	# Skip first spaces
	while curr().isspace(): step()
	# Read "macro" keyword
	keyword = ""
	while is_id(curr()): keyword += step()
	if keyword != "macro": raise Exception(f"Not a macro: {line}")
	# Skip spaces after "macro"
	while curr().isspace(): step()
	# Read macro name
	name = ""
	while curr() and not curr().isspace(): name += step()
	# Skip spaces after macro name
	while curr().isspace(): step()
	# Find all arguments
	args = []
	arg = ''
	while curr() and curr() != ';' and curr() != '{':
		# Collect identifier
		if is_id(curr()):
			arg += step()
		# Save the collected identifier
		elif curr() == ',':
			args.append(arg)
			arg = ''
			step()
		# Just push the '['
		elif curr() == '[':
			args.append(step())
		# Just push the identifier and get ']' ready to be pushed on next comma
		elif curr() == ']':
			args.append(arg)
			arg = step()
		# Just push the identifier and get '*' ready to be pushed on next comma
		elif curr() == '*':
			args.append(arg)
			arg = step()
		# Just skip whitespaces
		elif curr().isspace():
			step()
		# Something unexpected
		else:
			raise Exception(f"Unexpected symbol '{curr()}' at index #{i} " + 
			                f"in the macro declaration:\n'{line}'")
	if arg != '':
		args.append(arg)
	# Find a comment if any
	comment = ""
	while curr() and curr() != ';': step()
	if curr() == ';':
		step()
		while curr(): comment += step()
	# Find end of the macro
	end_of_macro = False
	while not end_of_macro:
		line = lines[line_idx]
		rbraces = re.finditer('}', line)
		for rbrace_match in rbraces:
			rbrace_idx = rbrace_match.start()
			if line[rbrace_idx - 1] != '\\':
				end_of_macro = True
		line_idx += 1
	# Process comment
	if comment != "":
		comment = comment.lstrip()
		if (len(comment) == 0):
			comment = "!!! EMPTY_COMMENT"
			warnings += f"{asm_file_name}:{line_idx + 1}: Empty comment in\n"
		if comment[0].islower():
			warnings += f"{asm_file_name}:{line_idx + 1}: Сomment sarting with lowercase\n"
	# Build the output
	line_span = line_idx - line_idx_orig + 1
	result = AsmMacro(asm_file_name, line_idx_orig, name, comment, args)
	return (line_span, result)

def get_declarations(asm_file_contents, asm_file_name):
	asm_file_name = asm_file_name.replace("./", "")
	kernel_structure[asm_file_name] = [ [], [], [], [], [] ]

	variable_pattern = re.compile(r'^\s*[\w\.]+\s+d[bwdq]\s+.*')
	macro_pattern = re.compile(r'^\s*macro\s+([\w]+).*')
	proc_pattern = re.compile(r'^\s*proc\s+([\w\.]+).*')
	label_pattern = re.compile(r'^(?!;)\s*([\w\.]+):.*')
	struct_pattern = re.compile(r'^\s*struct\s+([\w]+).*')

	line_idx = 0
	lines = asm_file_contents.splitlines()
	while line_idx < len(lines):
		line = lines[line_idx]

		if variable_pattern.match(line):
			(skip_lines, var) = parse_variable(asm_file_name, lines, line_idx)
			kernel_structure[asm_file_name][VARIABLES].append(var)
			line_idx += skip_lines
			continue

		match = macro_pattern.findall(line)
		if len(match) > 0:
			(skip_lines, macro) = parse_macro(asm_file_name, lines, line_idx)
			kernel_structure[asm_file_name][MACROS].append(macro)
			line_idx += skip_lines
			continue

		match = proc_pattern.findall(line)
		if len(match) > 0:
			proc_name = match[0]
			kernel_structure[asm_file_name][PROCEDURES].append(AsmFunction(line_idx + 1, proc_name))
			line_idx += 1
			continue

		match = label_pattern.findall(line)
		if len(match) > 0:
			label_name = match[0]
			# Don't count local labels
			if label_name[0] != '.':
				kernel_structure[asm_file_name][LABELS].append(AsmLabel(line_idx + 1, label_name))
				line_idx += 1
				continue

		match = struct_pattern.findall(line)
		if len(match) > 0:
			struct_name = match[0]
			kernel_structure[asm_file_name][STRUCTURES].append(AsmStruct(line_idx + 1, struct_name))
			end_of_struct = False
			while not end_of_struct:
				line = lines[line_idx]
				if re.match(r"^ends$", line) != None:
					end_of_struct = True
				line_idx += 1
			continue

		line_idx += 1

def handle_file(handled_files, asm_file_name, subdir = "."):
	if dump_symbols:
		print(f"Handling {asm_file_name}")
	handled_files.append(asm_file_name)
	try:
		asm_file_contents = open(asm_file_name, "r", encoding="utf-8").read()
	except:
		return
	get_declarations(asm_file_contents, asm_file_name)
	include_directive_pattern_1 = re.compile(r'include "(.*)"')
	include_directive_pattern_2 = re.compile(r'include \'(.*)\'')
	includes = include_directive_pattern_1.findall(asm_file_contents)
	includes += include_directive_pattern_2.findall(asm_file_contents)
	for include in includes:
		include = include.replace('\\', '/');
		full_path = subdir + '/' + include;
		if full_path not in handled_files:
			new_subdir = full_path.rsplit('/', 1)[0]
			handle_file(handled_files, full_path, new_subdir)
	return handled_files

kernel_files = []

handle_file(kernel_files, "./kernel.asm");

if dump_symbols:
	for source in kernel_structure:
		print(f"File: {source}")
		if len(kernel_structure[source][VARIABLES]) > 0:
			print(" Variables:")
			for variable in kernel_structure[source][VARIABLES]:
				print(f"  {variable.line}: {variable.name}")
		if len(kernel_structure[source][PROCEDURES]) > 0:
			print(" Procedures:")
			for procedure in kernel_structure[source][PROCEDURES]:
				print(f"  {procedure.line}: {procedure.name}")
		if len(kernel_structure[source][LABELS]) > 0:
			print(" Global labels:")
			for label in kernel_structure[source][LABELS]:
				print(f"  {label.line}: {label.name}")
		if len(kernel_structure[source][MACROS]) > 0:
			print(" Macroses:")
			for macro in kernel_structure[source][MACROS]:
				print(f"  {macro.line}: {macro.name}")
		if len(kernel_structure[source][STRUCTURES]) > 0:
			print(" Structures:")
			for struct in kernel_structure[source][STRUCTURES]:
				print(f"  {struct.line}: {struct.name}")

if print_stats:
	# Collect stats
	var_count = 0
	proc_count = 0
	label_count = 0
	macro_count = 0
	struct_count = 0

	for source in kernel_structure:
		var_count += len(kernel_structure[source][VARIABLES])
		proc_count += len(kernel_structure[source][PROCEDURES])
		label_count += len(kernel_structure[source][LABELS])
		macro_count += len(kernel_structure[source][MACROS])
		struct_count += len(kernel_structure[source][STRUCTURES])

	print(f"File count: {len(kernel_structure)}")
	print(f"Variable count: {var_count}")
	print(f"Procedures count: {proc_count}")
	print(f"Global labels count: {label_count}")
	print(f"Macroses count: {macro_count}")
	print(f"Structures count: {struct_count}")

print(f"Writing doumented sources to {doxygen_src_path}")

created_files = []

def write_something(source, somehing):
	full_path = doxygen_src_path + '/' + source
	# Remove the file on first access if it was created by previous generation
	if full_path not in created_files:
		if os.path.isfile(full_path):
			os.remove(full_path)
		created_files.append(full_path)
	# Only remove the file on 'clean_generated_stuff' flag (removed above, just return)
	if clean_generated_stuff: return
	# Create directories need for the file
	os.makedirs(os.path.dirname(full_path), exist_ok=True)
	f = open(full_path, "a")
	f.write(somehing)
	f.close()

def write_variable(source, variable):
	line = variable.line
	type = variable.type
	init = variable.init
	brief = variable.comment
	name = variable.name.replace(".", "_")
	something = (f"/**\n" +
	             f" * @brief {brief}\n" +
	             f" * @par Initial value\n" +
	             f" * {init}\n" +
	             f" * @par Source\n" +
	             f" * <a href='{link_root}/{source}#line-{line}'>{source}:{line}</a>\n" +
	             f" */\n" +
	             f"{type} {name};\n\n")
	write_something(source, something)

def write_procedure(source, line, name, brief = "Undocumented"):
	name = name.replace(".", "_")
	something = (f"/**\n" +
	             f" * @brief {brief}\n" +
	             f" * @par Source\n" +
	             f" * <a href='{link_root}/{source}#line-{line}'>{source}:{line}</a>\n" +
	             f" */\n" +
	             f"void {name}();\n\n")
	write_something(source, something)

def write_label(source, line, name, brief = "Undocumented"):
	name = name.replace(".", "_")
	something = (f"/**\n" +
	             f" * @brief {brief}\n" +
	             f" * @par Source\n" +
	             f" * <a href='{link_root}/{source}#line-{line}'>{source}:{line}</a>\n" +
	             f" */\n" +
	             f"void {name}();\n\n")
	write_something(source, something)

def write_macro(source, macro):
	if macro.comment == "": brief = "Undocumented"
	else: brief = macro.comment
	line = macro.line
	name = macro.name.replace(".", "_").replace("@", "_")
	# Construct arg list without '['s, ']'s and '*'s
	args = [arg for arg in macro.args if arg not in "[]*"]
	# Construct C-like arg list
	arg_list = ""
	if len(args) > 0:
		arg_list += '('
		argc = 0
		for arg in args:
			if argc != 0:
				arg_list += ", "
			arg_list += arg
			argc += 1
		arg_list += ')'

	something = (f"/**\n" +
	             f" * @def {name}\n" +
	             f" * @brief {brief}\n" +
	             f" * @par Source\n" +
	             f" * <a href='{link_root}/{source}#line-{line}'>{source}:{line}</a>\n" +
	             f" */\n#define {name}{arg_list}\n\n")
	write_something(source, something)

def write_structure(source, line, name, brief = "Undocumented"):
	name = name.replace(".", "_")
	something = (f"/**\n" +
	             f" * @struct {name}\n" +
	             f" * @brief {brief}\n" +
	             f" * @par Source\n" +
	             f" * <a href='{link_root}/{source}#line-{line}'>{source}:{line}</a>\n" +
	             f" */\nstruct {name}" + " {};\n\n")
	write_something(source, something)

i = 1
for source in kernel_structure:
	# Print progress: current/total
	print(f"{i}/{len(kernel_structure)} Writing {source}")
	# Write variables doxygen of the source file
	if len(kernel_structure[source][VARIABLES]) > 0:
		for variable in kernel_structure[source][VARIABLES]:
			write_variable(source, variable)
	if len(kernel_structure[source][PROCEDURES]) > 0:
		for procedure in kernel_structure[source][PROCEDURES]:
			write_procedure(source, procedure.line, procedure.name)
	if len(kernel_structure[source][LABELS]) > 0:
		for label in kernel_structure[source][LABELS]:
			write_label(source, label.line, label.name)
	if len(kernel_structure[source][MACROS]) > 0:
		for macro in kernel_structure[source][MACROS]:
			write_macro(source, macro)
	if len(kernel_structure[source][STRUCTURES]) > 0:
		for structure in kernel_structure[source][STRUCTURES]:
			write_structure(source, structure.line, structure.name)
	i += 1

if enable_warnings:
	open('asmxygen.txt', "w", encoding = "utf-8").write(warnings)
