#!/usr/bin/python3
# Copyright Magomed Kostoev
# Published under MIT license

import os
import shutil

from lib.network import download
from lib.logging import log
from lib.constants import tools, tools_workspace, tools_cache, tools_cache_kolibri_img

def generate_script_executing_script(script_to_execute):
    script_to_execute = script_to_execute.replace("\\", "\\\\")
    contents = ""
    contents += "from importlib.machinery import SourceFileLoader\n"
    contents += f"SourceFileLoader('__main__', '{script_to_execute}').load_module()\n"
    return contents

def create_workspace_script(name, script_to_execute):
    log(f"Installing {name}... ", end = "")

    script_contents = generate_script_executing_script(script_to_execute)
    with open(name, "w") as f:
        f.write(script_contents)

    log("Done")

if __name__ == "__main__":
    # Check if we have tup installed
    if shutil.which("tup") == None:
        print("Sorry, I haven't found tup")
        print("Possible solutions:")
        print("- Install tup")
        print("- Add tup installation folder to PATH")
        exit()
    # Initalize tup here
    os.system("tup init")
    # Create (in current directory) scripts that execute
    # the same named scripts from _tools/workspace
    tools_workspace_run_py = os.path.join(tools_workspace, "run.py")
    tools_workspace_build_py = os.path.join(tools_workspace, "build.py")
    create_workspace_script("run.py", tools_workspace_run_py)
    create_workspace_script("build.py", tools_workspace_build_py)

