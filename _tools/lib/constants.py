import os

### PATHS

# _tools/lib/constants.py
tools_lib_constants_py = os.path.abspath(__file__)

# _tools/lib
tools_lib = os.path.dirname(tools_lib_constants_py)

# _tools
tools = os.path.dirname(tools_lib)

# _tools/workspace
tools_workspace = os.path.join(tools, "workspace")

# _tools/cache
tools_cache = os.path.join(tools, "cache")

# _tools/cache/kolibri.img
tools_cache_kolibri_img = os.path.join(tools_cache, "kolibri.img")

