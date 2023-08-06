import os
import sys
import shlex
import shutil
import urllib.request
import subprocess

path_to_tools_workspace = os.path.dirname(os.path.abspath(__file__))
path_to_tools = os.path.dirname(path_to_tools_workspace)
sys.path.append(path_to_tools)

from workspace.build import build

from lib.builds import builds_get, builds_get_contents
from lib.makeflop import Floppy
from lib.platform import is_win32, path
from lib.logging import log, require_tools
from lib.constants import tools_cache_kolibri_img

# TODO: Move into _tools/lib
def run_qemu(start_dir = "workspace"):
    require_tools(("qemu-system-i386",))

    qemu_command = f"qemu-system-i386"
    flags = ""
    flags += "-L . " # IDK why it does not work without this
    flags += "-m 128 "
    flags += f"-drive format=raw,file={start_dir}/kolibri.img,index=0,if=floppy -boot a "
    flags += "-vga vmware "
    flags += "-net nic,model=rtl8139 -net user "
    if is_win32():
        qemu_full_path = shutil.which(qemu_command)
        qemu_directory = os.path.dirname(qemu_full_path)
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
    program_name = build()

    os.makedirs("workspace", exist_ok = True)

    # Create a copy of IMG
    kolibri_img = builds_get("eng/data/data/kolibri.img", "workspace/kolibri.img")

    # Open the IMG
    with open(kolibri_img, "rb") as img:
        img_data = img.read()
    img = Floppy(img_data)

    # Remove unuseful folders
    img.delete_path("GAMES")
    img.delete_path("DEMOS")
    img.delete_path("3D")

    # Insert faster kernel if no --compressed-kernel flag passed
    if "--compressed-kernel" not in sys.argv:
        new_kernel = builds_get_contents("eng/data/kernel/trunk/kernel.mnt.pretest")
        img.add_file_path("KERNEL.MNT", new_kernel)

    log("Moving program into kolibri image... ", end = "")
    with open(program_name, "rb") as file:
        file_data = file.read()
    if not img.add_file_path(program_name.upper(), file_data):
        print(f"Coudn't move {program_name} into IMG")
    log("Done")

    log("Adding program to autorun.dat... ", end = "")
    lines_to_add = bytes(f"\r\n/SYS/{program_name.upper()}\t\t""\t0\t# Your program", "ascii")
    autorun_dat = img.extract_file_path("SETTINGS\AUTORUN.DAT")
    place_for_new_lines = autorun_dat.index(b"\r\n/SYS/@TASKBAR")# b"\r\n### Hello, ASM World! ###")
    autorun_dat = autorun_dat[:place_for_new_lines] + lines_to_add + autorun_dat[place_for_new_lines:]
    img.delete_path("SETTINGS\AUTORUN.DAT")
    img.add_file_path("SETTINGS\AUTORUN.DAT", autorun_dat)
    log("Done")

    img.save(kolibri_img)

    run_qemu()
