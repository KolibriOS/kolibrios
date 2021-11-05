#!/usr/bin/python3
# Copyright 2021 KolibriOS Team
# Copyright 2021 Nekos Team
# Published under MIT License

import os
import sys
from importlib.machinery import SourceFileLoader
import timeit
import urllib.request
import subprocess

sys.path.append('test')
import common

root_dir = os.getcwd()
tests = []

def log(s, end = "\n"):
    print(s, end = end, flush = True)

def execute(s, mute = False):
    mute = ">/dev/null" if mute else ""
    code = os.system(f"{s}{mute}")
    if code:
        print(f"Command returned {code}: \"{s}\"")
        exit(-1)

def stage(name, command, mute = False):
    print(f"{name}... ", end = "")
    execute(command, mute = mute)
    print("Done.")

# Get IMG
if not os.path.exists("kolibri_test.img"):
    if len(sys.argv) == 1:
        execute("wget -q --show-progress http://builds.kolibrios.org/eng/data/data/kolibri.img -O kolibri_test.img")
    else:
        builds_eng = sys.argv[1]
        execute(f"cp {builds_eng}/data/data/kolibri.img kolibri_test.img")

# Remove old kernel (may fail if we removed it before so no check here)
os.system("mdel -i kolibri_test.img ::kernel.mnt > /dev/null")

# Check free space after kernel remove
free_clusters = int(subprocess.check_output("mdu -i kolibri_test.img :: -s", shell=True).split()[-1])
floppy_image_clusters = 2880
if floppy_image_clusters - free_clusters < 500:
    # Remove unuseful files from IMG if lesser than 500 sectors
    execute("mdeltree -i kolibri_test.img ::GAMES", mute = True)
    execute("mdeltree -i kolibri_test.img ::DEMOS", mute = True)
    execute("mdeltree -i kolibri_test.img ::3D", mute = True)

# Get test kernel
if not os.path.exists("kernel.mnt.pretest"):
    if len(sys.argv) == 1:
        execute("wget -q --show-progress http://builds.kolibrios.org/eng/data/kernel/trunk/kernel.mnt.pretest -O kernel.mnt.pretest")
    else:
        builds_eng = sys.argv[1]
        execute("cp {builds_eng}/data/kernel/trunk/kernel.mnt.pretest kernel.mnt.pretest", mute = True)

# Put the kernel into IMG
execute("mcopy -D o -i kolibri_test.img kernel.mnt.pretest ::kernel.mnt", mute = True)

# Collect tests from test folder (not recursively yet)
for test_folder in os.listdir("test"):
    test_folder_path = f"test/{test_folder}"
    test_file = f"{test_folder_path}/test.py"

    if not os.path.isdir(test_folder_path):
        continue

    if os.path.exists(test_file):
        tests.append(test_folder_path)

# Execute each test
test_number = 1
for test in tests:
    test_dir = f"{root_dir}/{test}"

    os.chdir(test_dir)
    print(f"[{test_number}/{len(tests)}] {test}... ", end = "", flush=True)
    start = timeit.default_timer()
    try:
        SourceFileLoader("test", "test.py").load_module().run()
    except common.TestTimeoutException:
        result = "TIMEOUT"
    except common.TestFailureException:
        result = "FAILURE"
    else:
        result = "SUCCESS"
    finish = timeit.default_timer()
    print(f"{result} ({finish - start:.2f} seconds)")
    os.chdir(root_dir)

    test_number += 1

