#!/usr/bin/python3
import sys

sys.path.append('../')
import common

def run():
    os = common.run()
    os.wait_for_debug_log("K : kernel SVN", timeout = 10)
    os.kill()

