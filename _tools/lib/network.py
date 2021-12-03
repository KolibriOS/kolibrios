import urllib.request

def download(link, path):
    log(f"Downloading {path}... ", end = "")
    urllib.request.urlretrieve(link, path)
    log("Done.")


