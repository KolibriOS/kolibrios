import sys
import os

path_to_tools_workspace = os.path.dirname(os.path.abspath(__file__))
path_to_tools = os.path.dirname(path_to_tools_workspace)
sys.path.append(path_to_tools)

from lib.tupfile_parser import parse as parse_tupfile

def build():
    if not os.path.exists("Tupfile.lua"):
        print("No Tupfile.lua, can't build anything")
        exit()

    tup_rules = parse_tupfile("Tupfile.lua")
    program_files = []
    for rule in tup_rules:
        # TODO: Manage source dependencies
        # TODO: Inform about tools required for the build
        os.system(rule.command)
        program_files += rule.output
    return program_files

if __name__ == "__main__":
    build()
