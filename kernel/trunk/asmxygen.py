import os
from glob import glob

"""
# Collect all .inc files
kernel_files = [y for x in os.walk(".") for y in glob(os.path.join(x[0], '*.inc'))]

to_remove = []
for i in range(len(kernel_files)):
	inc = kernel_files[i]
	# Remove files that aren't a part of the kernel
	if "bootloader" in inc or "sec_loader" in inc:
		to_remove.append(i)

for i in range(len(to_remove) - 1, -1, -1):
	kernel_files.pop(to_remove[i])

# Add main kernel file
kernel_files.append("kernel.asm")

# Add main kernel file
# TODO: Rename the file so it won't be an exception
kernel_files.append("fs/xfs.asm")
"""

import re

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

	variable_pattern = re.compile(r'^\s*([\w\.]+)\s+d[bwdq] .*')
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
			var_name = match[0]
			#print(f"Variable '{var_name}' at {line_idx + 1}")
			kernel_structure[asm_file_name][VARIABLES].append([ line_idx + 1, var_name ])
			line_idx += 1
			continue

		match = macro_pattern.findall(line)
		if len(match) > 0:
			macro_name = match[0]
			#print(f"Macro '{macro_name}' at {line_idx + 1}")
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
			#print(f"Procedure '{proc_name}' at {line_idx + 1}")
			kernel_structure[asm_file_name][PROCEDURES].append([ line_idx + 1, proc_name ])
			line_idx += 1
			continue

		match = label_pattern.findall(line)
		if len(match) > 0:
			label_name = match[0]
			# Don't count local labels
			if label_name[0] != '.':
				#print(f"Label '{label_name}' at {line_idx + 1}")
				kernel_structure[asm_file_name][LABELS].append([ line_idx + 1, label_name ])
				line_idx += 1
				continue

		match = struct_pattern.findall(line)
		if len(match) > 0:
			struct_name = match[0]
			#print(f"Structure '{struct_name}' at {line_idx + 1}")
			kernel_structure[asm_file_name][STRUCTURES].append([ line_idx + 1, struct_name ])
			end_of_struct = False
			while not end_of_struct:
				line = lines[line_idx]
				if re.match(r"^ends$", line) != None:
					end_of_struct = True
				line_idx += 1
			continue

		line_idx += 1

def get_includes(handled_files, asm_file_name, subdir = "."):
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
			get_includes(handled_files, full_path, new_subdir)
	return handled_files

kernel_files = []

get_includes(kernel_files, "./kernel.asm");

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

print(f"Variable count: {var_count}")
print(f"Procedures count: {proc_count}")
print(f"Global labels count: {label_count}")
print(f"Macroses count: {macro_count}")
print(f"Structures count: {struct_count}")

#for kernel_file in kernel_files:
#	print(kernel_file)
