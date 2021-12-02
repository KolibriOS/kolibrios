import sys
import os

path_to_lib = '../lib'
sys.path.append(path_to_lib)

import tupfile_parser

def build():
    if not os.path.exists("Tupfile.lua"):
        print("No Tupfile.lua, can't build anything")
        exit()

    tup_rules = tupfile_parser.parse("Tupfile.lua")
    program_files = []
    for rule in tup_rules:
        # TODO: Manage source dependencies
        # TODO: Inform about tools required for the build
        os.system(rule.command)
        program_files += rule.output
    return program_files

if __name__ == "__main__":
    build()