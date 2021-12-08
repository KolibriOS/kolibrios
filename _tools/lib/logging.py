import shutil

def log(s, end = "\n"):
    print(s, end = end, flush = True)

def require_tools(names):
    assert(type(names) == list or type(names) == tuple)
    for name in names:
        assert(type(name) == str)

    not_found = []
    for name in names:
        if shutil.which(name) is None:
            not_found.append(name)

    if len(not_found) != 0:
        log("Sorry, I can't find some tools:")
        for name in not_found:
            print(f"- {name}")
        exit(1)

