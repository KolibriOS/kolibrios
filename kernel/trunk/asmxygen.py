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

# Constants
link_root = "http://websvn.kolibrios.org/filedetails.php?repname=Kolibri+OS&path=/kernel/trunk"

# Parse arguments
parser = argparse.ArgumentParser()
parser.add_argument("-o", help="Doxygen output folder")
parser.add_argument("--clean", help="Remove generated files", action="store_true")
parser.add_argument("--dump", help="Dump all defined symbols", action="store_true")
parser.add_argument("--stats", help="Print symbol stats", action="store_true")
args = parser.parse_args()
doxygen_src_path = args.o if args.o else 'docs/doxygen'
clean_generated_stuff = args.clean
dump_symbols = args.dump
print_stats = args.stats

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

def get_declarations(asm_file_contents, asm_file_name):
	kernel_structure[asm_file_name] = [ [], [], [], [], [] ]

	variable_pattern = re.compile(r'^\s*([\w\.]+)\s+d([bwdq])\s+([^;]*)\s*([;].*)?')
	macro_pattern = re.compile(r'^\s*macro\s+([\w]+).*')
	proc_pattern = re.compile(r'^\s*proc\s+([\w\.]+).*')
	label_pattern = re.compile(r'^(?!;)\s*([\w\.]+):.*')
	struct_pattern = re.compile(r'^\s*struct\s+([\w]+).*')

	line_idx = 0
	lines = asm_file_contents.splitlines()
	while line_idx < len(lines):
		line = lines[line_idx]

		match = variable_pattern.findall(line)
		if len(match) > 0:
			(var_name, var_type, var_init, var_comm) = match[0]
			if var_comm == "":
				var_comm = "Undocumented"
			else:
				var_comm = var_comm[1:].lstrip()
				if (len(var_comm) == 0):
					var_comm = "!!! EMPTY_COMMENT"
				if var_comm[0].islower(): var_comm = "!!! LOWERCASE COMMENT " + var_comm
			if var_type == "b": var_type = "byte"
			if var_type == "w": var_type = "word"
			if var_type == "d": var_type = "dword"
			if var_type == "q": var_type = "qword"
			kernel_structure[asm_file_name][VARIABLES].append([ line_idx + 1, var_name, var_type, var_init, var_comm ])
			line_idx += 1
			continue

		match = macro_pattern.findall(line)
		if len(match) > 0:
			macro_name = match[0]
			kernel_structure[asm_file_name][MACROS].append([ line_idx + 1, macro_name ])
			end_of_macro = False
			while not end_of_macro:
				line = lines[line_idx]
				rbraces = re.finditer('}', line)
				for rbrace_match in rbraces:
					rbrace_idx = rbrace_match.start()
					if line[rbrace_idx - 1] != '\\':
						end_of_macro = True
				line_idx += 1
			continue

		match = proc_pattern.findall(line)
		if len(match) > 0:
			proc_name = match[0]
			kernel_structure[asm_file_name][PROCEDURES].append([ line_idx + 1, proc_name ])
			line_idx += 1
			continue

		match = label_pattern.findall(line)
		if len(match) > 0:
			label_name = match[0]
			# Don't count local labels
			if label_name[0] != '.':
				kernel_structure[asm_file_name][LABELS].append([ line_idx + 1, label_name ])
				line_idx += 1
				continue

		match = struct_pattern.findall(line)
		if len(match) > 0:
			struct_name = match[0]
			kernel_structure[asm_file_name][STRUCTURES].append([ line_idx + 1, struct_name ])
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
				print(f"  {variable[0]}: {variable[1]}")
		if len(kernel_structure[source][PROCEDURES]) > 0:
			print(" Procedures:")
			for procedure in kernel_structure[source][PROCEDURES]:
				print(f"  {procedure[0]}: {procedure[1]}")
		if len(kernel_structure[source][LABELS]) > 0:
			print(" Global labels:")
			for label in kernel_structure[source][LABELS]:
				print(f"  {label[0]}: {label[1]}")
		if len(kernel_structure[source][MACROS]) > 0:
			print(" Macroses:")
			for macro in kernel_structure[source][MACROS]:
				print(f"  {macro[0]}: {macro[1]}")
		if len(kernel_structure[source][STRUCTURES]) > 0:
			print(" Structures:")
			for struct in kernel_structure[source][STRUCTURES]:
				print(f"  {struct[0]}: {struct[1]}")

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

def write_variable(source, line, name, type, init, brief):
	source = source.replace("./", "")
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
	name = name.replace(".", "_")
	f = open(full_path, "a")
	f.write(f"/**\n")
	f.write(f" * @brief {brief}\n")
	f.write(f" * @par Initial value\n")
	f.write(f" * {init}\n")
	f.write(f" * @par Source\n")
	f.write(f" * <a href='{link_root}/{source}#line-{line}'>{source}:{line}</a>\n")
	f.write(f" */\n")
	f.write(f"{type} {name};\n\n")
	f.close()

i = 1
for source in kernel_structure:
	# Print progress: current/total
	print(f"{i}/{len(kernel_structure)} Writing {source}")
	# Write variables doxygen of the source file
	if len(kernel_structure[source][VARIABLES]) > 0:
		for variable in kernel_structure[source][VARIABLES]:
			write_variable(source, variable[0], variable[1], variable[2], variable[3], variable[4])
	i += 1
