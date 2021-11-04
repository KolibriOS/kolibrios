# Copyright 2021 KolibriOS Team
# Copyright 2021 Nekos Team
# Published under MIT License

import io
import os
import subprocess
import timeit
import time
import shlex
import signal

class TestTimeoutException(Exception):
    pass

class TestFailureException(Exception):
    pass

class Qemu:
    def __init__(self, popen):
        self.popen = popen
        self.wait() # qemu needs time to create debug.log file

    def wait_for_debug_log(self, needle, timeout = 1):
        needle = bytes(needle, "utf-8")
        start = timeit.default_timer()
        stdout = open("debug.log", "rb")
        log = b""

        # While no timeout, read and search logs
        while timeit.default_timer() - start < timeout:
            log += stdout.read(1)
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
        os.killpg(os.getpgid(self.popen.pid), signal.SIGTERM)

    def failure(self):
        self.kill()
        raise TestFailureException()

    def timeout(self):
        self.kill()
        raise TestTimeoutException()

    def wait(self, seconds = 0.25):
        time.sleep(seconds)

def run():
    s = f"qemu-system-i386 -nographic -L . -m 128 -drive format=raw,file=../../kolibri_test.img,index=0,if=floppy -boot a -vga vmware -net nic,model=rtl8139 -net user -soundhw ac97 -debugcon file:debug.log"
    a = shlex.split(s)
    popen = subprocess.Popen(a, bufsize = 0, stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL, start_new_session = True)
    return Qemu(popen)

