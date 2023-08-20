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
        install_command = 'sudo apt install'
        for _, package in not_exists:
            install_command += f' {package}'
        log(f"Try to install with:\n  {install_command}\n")
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
    print("\nRunning QEMU tests.")
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


def test_umka():
    class Test:
        def __init__(self, path, deps):
            self.path = os.path.realpath(path)
            self.name = os.path.basename(path)
            self.deps = deps
            filename_no_ext = os.path.splitext(self.path)[0]
            self.ref_log = f"{filename_no_ext}.ref.log"
            self.out_log = f"{filename_no_ext}.out.log"
            self.ref_png = f"{filename_no_ext}.ref.png"
            self.out_png = f"{filename_no_ext}.out.png"
            self.log_diff = f"{filename_no_ext}.log.diff"
            self.check_png = os.path.exists(self.ref_png)

    def find_tests():
        def find_test_dependencies(umka_shell_command_file):
            # TODO: Cache the result to not parse tests on each run.
            deps = set()
            with open(umka_shell_command_file) as f:
                test_dir = os.path.dirname(umka_shell_command_file)
                for line in f:
                    parts = line.split()
                    for dependant in ("disk_add", "ramdisk_init"):
                       try:
                            idx = parts.index(dependant)
                            relative_img_path = parts[idx + 1]
                            dep_path = f"{test_dir}/{relative_img_path}"
                            deps.add(os.path.realpath(dep_path))
                       except:
                          pass
            return tuple(deps)

        tests = []
        for umka_shell_command_file in os.listdir("umka/test"):
            umka_shell_command_file = f"umka/test/{umka_shell_command_file}"
            if not umka_shell_command_file.endswith(".t"):
                continue
            if not os.path.isfile(umka_shell_command_file):
                continue
            deps = find_test_dependencies(umka_shell_command_file)
            tests.append(Test(umka_shell_command_file, deps))

        return tests

    print("\nCollecting UMKa tests.", flush = True)
    tests = find_tests()
    # Excluded: #acpi_.
    tags_to_tests = ("#xfs_", "#xfsv5_", "#exfat_", "#fat_", "#ext_", "#s05k_",
                     "#s4k_", "#f30_", "#f70_", "#f70s0_", "#f70s1_", "#f70s5_",
                     "#lookup_", "#bug_", "#xattr_", "#unicode_", "#draw_",
                     "#coverage_", "#i40_", "#net_", "#arp_", "#input_",
                     "#gpt_", "#uevent_")
    tests_to_run = []
    for test in tests:
        # If none of required tags are in the test name - skip it.
        for tag in tags_to_tests:
            if tag in test.name:
                break
        else:
            continue

        # Check test dependencies.
        unmet_deps = []
        for dep in test.deps:
            if not os.path.exists(dep):
                unmet_deps.append(dep)

        if len(unmet_deps) > 0:
            print(f"*** WARNING: Test {test.name} has been skipped, unmet dependencies:")
            for dep in unmet_deps:
                print(f"- {os.path.basename(dep)}")
            continue

        tests_to_run.append(test)

    failed_tests = []
    test_count = len(tests_to_run)
    test_i = 1
    print("\nRunning UMKa tests.")
    for test in tests_to_run:
        print(f"[{test_i}/{test_count}] Running test {test.name}... ", end = "", flush = True)
        if os.system(f"(cd umka/test && ../umka_shell -ri {test.path} -o {test.out_log})") != 0:
            print("ABORT")
        else:
            fail_reasons = []
            if not filecmp.cmp(test.out_log, test.ref_log):
                fail_reasons.append("log")
            if test.check_png and not filecmp.cmp(test.out_png, test.ref_png):
                fail_reasons.append("png")
            if fail_reasons:
                failed_tests.append((test, fail_reasons))
                print("FAILURE")
            else:
                print("SUCCESS")
        test_i += 1

    if len(failed_tests) != 0:
        print("\nFailed UMKa tests:")
        for failed_test in failed_tests:
            test = failed_test[0]
            reasons = failed_test[1]
            message = f"- {test.name}"
            if "log" in reasons:
                os.system(f"git --no-pager diff --no-index {test.ref_log} {test.out_log} > {test.log_diff}")
                message += f"\n  - logs differ: {test.log_diff}"
            if "png" in reasons:
                message += f"\n  - pngs are different:\n"
                message += f"    - {test.ref_png}\n"
                message += f"    - {test.out_png}"
            print(message)


def build_umka():
    print("\nBuilding UMKa... ", end = "", flush = True)
    env = os.environ
    env["KOLIBRIOS"] = os.path.abspath("../../")
    env["HOST"] = "linux"
    env["CC"] = "clang"
    popen = subprocess.Popen(shlex.split("make --silent -C umka umka_shell default.skn"), env = env)
    if popen.wait() != 0:
        subprocess.Popen(shlex.split("make --no-print-directory -C umka clean umka_shell default.skn"), env = env)
    if os.system("make --silent -C umka/apps board_cycle") != 0:
        os.system("make --no-print-directory -C umka/apps clean board_cycle")
    if os.system("make --silent -C umka/tools all") != 0:
        os.system("make --no-print-directory -C umka/tools clean all")
    print("Done.")

    print("\nGenerating images for UMKa tests.", flush = True)
    os.system("(cd umka/img && sudo ./gen.sh)")


def download_umka():
	if not os.path.exists("umka"):
		if os.system("git clone https://github.com/KolibriOS/umka") != 0:
			print("Couldn't clone UMKa repo")
			exit()


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
        test_umka()
    tests = collect_tests()
    serial_executor_thread = run_tests_serially(tests, root_dir)
    serial_executor_thread.join()
