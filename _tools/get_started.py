#!/usr/bin/python3
# Copyright Magomed Kostoev
# Published under MIT license

import os

def log(s, end = "\n"):
    print(s, end = end, flush = True)

def install_python_script(src, dst, tools_lib):
    log(f"Copying {src}... ", end = "")

    with open(src) as src_file:
        script = src_file.read()
    tools_lib_escaped = tools_lib.replace("\\", "\\\\")
    repl_from = "path_to_lib = '../lib'"
    repl_to = f"path_to_lib ='{tools_lib_escaped}'"
    script = script.replace(repl_from, repl_to, 1)
    with open(dst, "w") as dst_file:
        dst_file.write(script)

    log(f"Done")

if __name__ == "__main__":
    tools_get_started_py = os.path.abspath(__file__)
    tools = os.sep.join(tools_get_started_py.split(os.sep)[:-1])
    tools_lib = os.sep.join([tools, "lib"])
    tools_workspace = os.sep.join([tools, "workspace"])
    # Copy scripts from _tools/workspace to current folder, but let them know
    # where the _tools/lib is (change their value of tools_lib variable)
    tools_workspace_run_py = os.sep.join([tools_workspace, "run.py"])
    tools_workspace_build_py = os.sep.join([tools_workspace, "build.py"])
    install_python_script(tools_workspace_run_py, "run.py", tools_lib)
    install_python_script(tools_workspace_build_py, "build.py", tools_lib)
