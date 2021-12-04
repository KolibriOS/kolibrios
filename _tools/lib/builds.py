import os
import shutil

from .network import download
from .constants import tools_cache

def builds_get(path, output_path = None):
    url = f"http://builds.kolibrios.org/{path}"
    cached_path = f"{tools_cache}/builds.kolibrios.org/{path}"
    os.makedirs(os.path.dirname(cached_path), exist_ok = True)
    download(url, cached_path, skip_exist = True)
    if output_path != None:
        shutil.copyfile(cached_path, output_path)
        return output_path
    return cached_path

def builds_get_contents(path):
    output_path = builds_get(path)
    with open(output_path, "rb") as f:
        return f.read()

