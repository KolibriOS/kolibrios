#!/usr/bin/python3
import os
import sys
import shutil

if len(sys.argv) < 2 or sys.argv[1] != "--remove-everything":
    print(f"Please call `{sys.argv[0]} --remove-everything` if you really want to remove all your workspace files")
    exit()

# Remove workspace folder
shutil.rmtree("workspace", ignore_errors = True)

# Remove tup database
shutil.rmtree(".tup", ignore_errors = True)

# TODO: Make build.py remove the stuff it built

# Remove files copied from _tools/workspace
tools = os.path.dirname(os.path.realpath(__file__))
tools_workspace = os.path.join(tools, "workspace")
for copied_script in os.listdir(tools_workspace):
    if os.path.exists(copied_script):
        os.remove(copied_script)

