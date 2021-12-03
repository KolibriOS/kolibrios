import os
import sys

def path(*args):
    return os.sep.join(args)

def is_win32():
    return True if sys.platform == "win32" else False

def is_linux():
    return True if sys.platform == "linux" or sys.platform == "linux2" else False

def is_osx():
    return True if sys.platform == "darwin" else False


