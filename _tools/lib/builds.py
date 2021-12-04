import os

from .network import download
from .constants import tools_cache

def builds_get(path):
    url = f"http://builds.kolibrios.org/{path}"
    output_path = f"{tools_cache}/builds.kolibrios.org/{path}"
    output_dir = os.path.dirname(output_path)
    os.makedirs(output_dir, exist_ok = True)
    download(url, output_path, skip_exist = True)
    return output_path

def builds_get_contents(path):
    output_path = builds_get(path)
    with open(output_path, "rb") as f:
        return f.read()

