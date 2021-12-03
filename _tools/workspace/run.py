import os
import sys
import shlex
import shutil
import urllib.request
import subprocess

import build

path_to_tools = '..'
sys.path.append(path_to_tools)

from lib.makeflop import Floppy
from lib.platform import is_win32, path
from lib.logging import log
from lib.network import download

# TODO: Move into _tools/lib
def get_file_directory(path):
    path = path.replace("\\", "/")
    if "/" in path:
        folder = "/".join(path.split("/")[:-1])
        if folder == "":
            return "/" # It was a file in the root folder
        return folder
    else:
        return "." # Just a filename, let's return current folder

# TODO: Move into _tools/lib
def run_qemu(start_dir = "workspace"):
    qemu_command = f"qemu-system-i386"
    flags = ""
    flags += "-L . " # IDK why it does not work without this
    flags += "-m 128 "
    flags += f"-drive format=raw,file={start_dir}/kolibri.img,index=0,if=floppy -boot a "
    flags += "-vga vmware "
    flags += "-net nic,model=rtl8139 -net user "
    flags += "-soundhw ac97 "
    if is_win32():
        qemu_full_path = shutil.which(qemu_command)
        qemu_directory = get_file_directory(qemu_full_path)
        flags += f"-L {qemu_directory} "
    s = f"{qemu_command} {flags}"
    qemu_stdout = open(f"{start_dir}/qemu_stdout.log", "w")
    qemu_stderr = open(f"{start_dir}/qemu_stderr.log", "w")
    if is_win32():
        return subprocess.Popen(s, bufsize = 0, stdout = qemu_stdout, stderr = qemu_stderr, stdin = subprocess.DEVNULL, shell = True, start_new_session = True)
    else:
        a = shlex.split(s)
        return subprocess.Popen(a, bufsize = 0, stdout = qemu_stdout, stderr = qemu_stderr, stdin = subprocess.DEVNULL, start_new_session = True)

if __name__ == "__main__":
    program_files = build.build()

    os.makedirs("workspace", exist_ok = True)

    if not os.path.exists("workspace/kolibri.img"):
        img_url = "http://builds.kolibrios.org/eng/data/data/kolibri.img"
        download(img_url, "workspace/kolibri.img")

    # Open the IMG
    with open("workspace/kolibri.img", "rb") as img:
        img_data = img.read()
    img = Floppy(img_data)

    # Remove unuseful folders
    img.delete_path("GAMES")
    img.delete_path("DEMOS")
    img.delete_path("3D")

    log("Moving program files into kolibri image... ", end = "")
    for file_name in program_files:
        with open(file_name, "rb") as file:
            file_data = file.read()
        if not img.add_file_path(file_name, file_data):
            print(f"Coudn't move {file_name} into IMG")
    img.save("workspace/kolibri.img")
    log("Done")

    # TODO: Autorun
    run_qemu()
