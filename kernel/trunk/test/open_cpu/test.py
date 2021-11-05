#!/usr/bin/python3
import sys

sys.path.append('../')
import common

def run():
    # Just an example test, disabled cause it's 3+ seconds long
    return
    test = common.run()
    test.wait(3)
    test.take_screenshot("before.ppm")
    test.send_keys("ctrl-alt-delete")
    test.wait(0.25)
    test.take_screenshot("after.ppm")
    test.images_diff("before.ppm", "after.ppm", expect = True)
    test.kill()

