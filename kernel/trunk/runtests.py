#!/usr/bin/python3
# Copyright 2021 Magomed Kostoev
# Published under MIT License

import os
import sys
import urllib
from importlib.machinery import SourceFileLoader
from shutil import which
import timeit
import urllib.request
import subprocess
from threading import Thread
import filecmp
import traceback
import shlex

sys.path.append('test')
import common

use_umka = False


def log(s, end="\n"):
    print(s, end=end, flush=True)


def check_retcode(command):
    popen = subprocess.Popen(shlex.split(command))
    return popen.wait()


def execute(s, mute=False):
    mute = ">/dev/null" if mute else ""
    code = os.system(f"{s}{mute}")
    if code:
        print(f"Command returned {code}: \"{s}\"")
        exit(-1)


def download(link, path):
    log(f"Downloading {path}... ", end="")
    urllib.request.urlretrieve(link, path)
    log("Done.")


def tool_exists(name):
    assert(type(name) == str)
    return which(name) is not None


def check_tools(tools):
    assert(type(tools) == list)
    for name_package_pair in tools:
        assert(type(name_package_pair) == list)
        assert(len(name_package_pair) == 2)
        assert(type(name_package_pair[0]) == str)
        assert(type(name_package_pair[1]) == str)

    not_exists = []
    for name, package in tools:
        if not tool_exists(name):
            not_exists.append((name, package))
    if len(not_exists) != 0:
        log("Sorry, I can't find some tools:")

        header_name = "Name"
        header_package = "Package (probably)"

        max_name_len = len(header_name)
        max_package_name_len = len(header_package)
        for name, package in not_exists:
            if len(package) > max_package_name_len:
                max_package_name_len = len(package)
            if len(name) > max_name_len:
                max_name_len = len(name)

        def draw_row(name, package):
            log((f" | {name.ljust(max_name_len)}" +
                 f" | {package.ljust(max_package_name_len)} |"))

        def draw_line():
            log(f" +-{'-' * max_name_len}-+-{'-' * max_package_name_len}-+")

        draw_line()
        draw_row(header_name, header_package)
        draw_line()
        for name, package in not_exists:
            draw_row(name, package)
        draw_line()
        exit()


def prepare_test_img():
    # TODO: Always recompile the kernel (after build system is done?)
    # Get IMG
    if not os.path.exists("kolibri_test.img"):
        if len(sys.argv) == 1:
            download("http://builds.kolibrios.org/eng/data/data/kolibri.img",
                     "kolibri_test.img")
        else:
            builds_eng = sys.argv[1]
            execute(f"cp {builds_eng}/data/data/kolibri.img kolibri_test.img")

    # Open the IMG
    with open("kolibri_test.img", "rb") as img:
        img_data = img.read()
    img = common.Floppy(img_data)

    # Remove unuseful folders
    img.delete_path("GAMES")
    img.delete_path("DEMOS")
    img.delete_path("3D")

    # Get test kernel
    if not os.path.exists("kernel.mnt.pretest"):
        if len(sys.argv) == 1:
            if check_retcode("tup dbconfig") != 0:
                execute("tup init")
            execute("tup kernel.mnt.pretest")
        else:
            builds_eng = sys.argv[1]
            kernel_mnt_pretest_subpath = "data/kernel/trunk/kernel.mnt.pretest"
            kernel_mnt_pretest = f"{builds_eng}/{kernel_mnt_pretest_subpath}"
            execute(f"cp {kernel_mnt_pretest} kernel.mnt.pretest", mute=True)

    # Put the kernel into IMG
    with open("kernel.mnt.pretest", "rb") as kernel_mnt_pretest:
        kernel_mnt_pretest_data = kernel_mnt_pretest.read()
    img.add_file_path("KERNEL.MNT", kernel_mnt_pretest_data)
    img.save("kolibri_test.img")


def collect_tests():
    tests = []

    # Collect tests from test folder (not recursively yet)
    for test_folder in os.listdir("test"):
        test_folder_path = f"test/{test_folder}"
        test_file = f"{test_folder_path}/test.py"

        if not os.path.isdir(test_folder_path):
            continue

        if os.path.exists(test_file):
            tests.append(test_folder_path)
    return tests


def run_tests_serially_thread(test, root_dir):
    errors = []
    test_number = 1
    for test in tests:
        test_dir = f"{root_dir}/{test}"

        print(f"[{test_number}/{len(tests)}] {test}... ", end="", flush=True)
        start = timeit.default_timer()
        try:
            loader = SourceFileLoader("test", f"{test_dir}/test.py")
            loader.load_module().run(root_dir, test_dir)
        except common.TestException as exception:
            result = exception.kind()
            errors.append((test, exception))
        else:
            result = "SUCCESS"
        finish = timeit.default_timer()
        print(f"{result} ({finish - start:.2f} seconds)")

        test_number += 1
    if len(errors) != 0:
        print("Some tests failed:")
        for error in errors:
            test, exception = error
            print(f"\n{test}: {str(exception)}\n\nTraceback:")
            traceback.print_tb(exception.__traceback__)
            print(f"\nQemu command:\n  {exception.cmd()}\n")


def run_tests_serially(tests, root_dir):
    thread = Thread(target=run_tests_serially_thread, args=(tests, root_dir))
    thread.start()
    return thread


def build_umka():
    kolibrios_dir = os.path.abspath("../../")
    env = os.environ
    env["KOLIBRIOS"] = kolibrios_dir
    env["HOST"] = "linux"
    env["CC"] = "clang"
    popen = subprocess.Popen(shlex.split("make -C umka umka_shell"), env = env)
    if popen.wait() != 0:
        subprocess.Popen(shlex.split("make -C umka clean"), env = env)


def download_umka():
	if not os.path.exists("umka"):
		if os.system("git clone https://github.com/KolibriOS/umka") != 0:
			print("Couldn't clone UMKa repo")
			exit()


def download_umka_imgs():
	imgs = [
		"fat32_test0.img",
		"jfs.img",
		"kolibri.img",
		"xfs_borg_bit.img",
		"xfs_v4_btrees_l2.img",
		"xfs_v4_files_s05k_b4k_n8k.img",
		"xfs_v4_ftype0_s05k_b2k_n8k_xattr.img",
		"xfs_v4_ftype0_s05k_b2k_n8k.img",
		"xfs_v4_ftype0_s4k_b4k_n8k.img",
		"xfs_v4_ftype1_s05k_b2k_n8k.img",
		"xfs_v4_unicode.img",
		"xfs_v4_xattr.img",
		"xfs_v5_files_s05k_b4k_n8k.img",
		"xfs_v5_ftype1_s05k_b2k_n8k.img",
	]

	for img in imgs:
		if not os.path.exists(f"umka/img/{img}"):
			download(f"http://ftp.kolibrios.org/users/Boppan/img/{img}",
					 f"umka/img/{img}")

if __name__ == "__main__":
    root_dir = os.getcwd()

    # Check available tools
    tools = [
        ["qemu-system-i386", "qemu-system-x86"],
        ["fasm", "fasm"],
        ["tup", "tup"],
    ]
    if use_umka:
        tools.append(["git", "git"])
        tools.append(["make", "make"])
    check_tools(tools)

    prepare_test_img()
    if use_umka:
        download_umka()
        build_umka()
    tests = collect_tests()
    serial_executor_thread = run_tests_serially(tests, root_dir)
    serial_executor_thread.join()
