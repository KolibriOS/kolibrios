import sys
import os

path_to_tools_workspace = os.path.dirname(os.path.abspath(__file__))
path_to_tools = os.path.dirname(path_to_tools_workspace)
sys.path.append(path_to_tools)

from lib.tupfile_parser import parse_tupfile_outputs

def build():
    os.system("tup")
    outputs = parse_tupfile_outputs("Tupfile.lua")
    for name in outputs:
        if name.endswith(".inc"):
            continue
        return name

if __name__ == "__main__":
    build()
