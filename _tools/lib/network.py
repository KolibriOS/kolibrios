import os
import urllib.request
from .logging import log

def download(link, path, skip_exist = False):
    if skip_exist and os.path.exists(path):
        return
    log(f"Downloading {path}... ", end = "")
    urllib.request.urlretrieve(link, path)
    log("Done.")

