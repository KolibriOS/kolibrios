import sys
import os

path_to_tools_workspace = os.path.dirname(os.path.abspath(__file__))
path_to_tools = os.path.dirname(path_to_tools_workspace)
sys.path.append(path_to_tools)

from lib.tupfile_parser import parse_required_compilers, parse_tupfile_outputs
from lib.logging import require_tools

def get_executable_file(output_file_list):
    for name in output_file_list:
        if name.endswith(".inc"):
            continue
        return name

def build():
    required_compilers = parse_required_compilers("Tupfile.lua")
    require_tools(required_compilers)
    os.system("tup")
    output_file_list = parse_tupfile_outputs("Tupfile.lua")
    return get_executable_file(output_file_list)

def clean():
    output_file_list = parse_tupfile_outputs("Tupfile.lua")
    for output_file in output_file_list:
        if os.path.exists(output_file):
            os.remove(output_file)

def main(argv):
    if len(argv) == 2 and argv[1] == "clean":
        clean()
    else:
        build()

if __name__ == "__main__":
    main(sys.argv)
