import os

tools_lib_constants_py = os.path.abspath(__file__)
tools_lib = os.path.dirname(tools_lib_constants_py)
tools = os.path.dirname(tools_lib)
tools_cache = os.path.join(tools, "cache")
tools_cache_kolibri_img = os.path.join(tools_cache, "kolibri.img")

