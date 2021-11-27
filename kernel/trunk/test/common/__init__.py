# Copyright 2021 Magomed Kostoev
# Published under MIT License

import io
import os
import sys
import subprocess
import timeit
import time
import shlex
import signal
import shutil
from . makeflop import Floppy

def is_win32():
    return True if sys.platform == "win32" else False

def is_linux():
    return True if sys.platform == "linux" or sys.platform == "linux2" else False

def  is_osx():
    return True if sys.platform == "darwin" else False

class TestTimeoutException(Exception):
    pass

class TestFailureException(Exception):
    pass

class Qemu:
    def __init__(self, popen, debug_log):
        self.popen = popen
        # Qemu needs time to create debug.log file
        while not os.path.exists(debug_log):
            self.wait()
        self.debug = open(debug_log, "rb")

    def wait_for_debug_log(self, needle, timeout = 1):
        needle = bytes(needle, "utf-8")
        start = timeit.default_timer()
        log = b""

        # While no timeout, read and search logs
        while timeit.default_timer() - start < timeout:
            log += self.debug.read(1)
            if needle in log:
                return

            # We don't have to read whole logs to find the neddle
            # If we read len(needle) * 2 bytes of log then we
            # already can say that if there's no needle in the data
            # then it can't be in first len(needle) bytes of the data
            # so first len(needle) bytes of saved logs may be thrown away
            #
            # So we consume lessser memory and don't search all the previous
            # logs every single time
            if len(log) > len(needle) * 2:
                log = log[len(needle):]

        self.timeout()

    def kill(self):
        if is_win32():
            # FIXME: This is shit, isn't there anything better?
            subprocess.Popen(f"TASKKILL /F /PID {self.popen.pid} /T", stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL, stdin = subprocess.DEVNULL)
        else:
            os.killpg(os.getpgid(self.popen.pid), signal.SIGTERM)

    def failure(self):
        self.kill()
        raise TestFailureException()

    def timeout(self):
        self.kill()
        raise TestTimeoutException()

    def wait(self, seconds = 0.25):
        time.sleep(seconds)

def get_file_directory(path):
    path = path.replace("\\", "/")
    if "/" in path:
        folder = "/".join(path.split("/")[:-1])
        if folder == "":
            return "/" # It was a file in the root folder
        return folder
    else:
        return "." # Just a filename, let's return current folder

def run_qemu(root_dir, test_dir, debug_log):
    # Make local copy of IMG, so we will be able to run the test in parallel
    if os.path.exists(f"{test_dir}/kolibri_test.img"): # If previous test run interrupted the file may be busy
        os.remove(f"{test_dir}/kolibri_test.img")
    shutil.copyfile(f"{root_dir}/kolibri_test.img", f"{test_dir}/kolibri_test.img")
    qemu_command = f"qemu-system-i386"
    flags = ""
    flags += "-nographic " # Makes it faster
    flags += f"-debugcon file:{debug_log} " # 0xe9 port output
    flags += "-L . " # IDK why it does not work without this
    flags += "-m 128 "
    flags += f"-drive format=raw,file={test_dir}/kolibri_test.img,index=0,if=floppy -boot a "
    flags += "-vga vmware "
    flags += "-net nic,model=rtl8139 -net user "
    flags += "-soundhw ac97 "
    if is_win32():
        qemu_full_path = shutil.which(qemu_command)
        qemu_directory = get_file_directory(qemu_full_path)
        flags += f"-L {qemu_directory} "
    s = f"{qemu_command} {flags}"
    qemu_stdout = open(f"{test_dir}/qemu_stdout.log", "w")
    qemu_stderr = open(f"{test_dir}/qemu_stderr.log", "w")
    if is_win32():
        return subprocess.Popen(s, bufsize = 0, stdout = qemu_stdout, stderr = qemu_stderr, stdin = subprocess.DEVNULL, shell = True, start_new_session = True)
    else:
        a = shlex.split(s)
        return subprocess.Popen(a, bufsize = 0, stdout = qemu_stdout, stderr = qemu_stderr, stdin = subprocess.DEVNULL, start_new_session = True)

def run(root_dir, test_dir):
    debug_log = f"{test_dir}/debug.log"
    if os.path.exists(debug_log):
        os.remove(debug_log)
    popen = run_qemu(root_dir, test_dir, debug_log)
    return Qemu(popen, debug_log)

