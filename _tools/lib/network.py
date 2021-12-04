import os
import urllib.request
from .logging import log

def download(link, path):
    log(f"Downloading {path}... ", end = "")
    urllib.request.urlretrieve(link, path)
    log("Done.")

def download_if_not_exist(link, path):
    if not os.path.exists(path):
        download(link, path)

