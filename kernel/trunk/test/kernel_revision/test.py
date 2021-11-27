#!/usr/bin/python3
import sys

sys.path.append('../')
import common

def run(root_dir, test_dir):
    os = common.run(root_dir, test_dir)
    os.wait_for_debug_log("K : kernel SVN", timeout = 10)
    os.kill()

