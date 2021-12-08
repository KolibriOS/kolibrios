# Copyright Magomed Kostoev
# Published under MIT license

block_beginner2finisher = {
    "(": ")",
    "{": "}",
    "\"": "\"",
}

def get_to(src, ptr, c, backwards = False):
    while src[ptr] != c:
        if not backwards:
            ptr += 1
        else:
            ptr -= 1
    return ptr

# Assuming we are at beginning of some block (string,
# parenthesed expression, {}-like thing), get to end
# of the block (also handles the blocks openned in the
# way to the block ending character. So it's not just
# stupid searching for block closing character.
def get_to_block_finisher(src, ptr):
    assert(src[ptr] in block_beginner2finisher)

    block_beginner = src[ptr]
    ptr += 1
    while src[ptr] != block_beginner2finisher[block_beginner]:
        # If any block starts here - get to its end and then continue
        if src[ptr] in block_beginner2finisher:
            ptr = get_to_block_finisher(src, ptr)
        ptr += 1
    return ptr

def get_strnig(src, ptr):
    # Strings starts with "\""
    assert(src[ptr] == "\"")

    result = ""
    # Skip first "\"" of the string
    ptr += 1
    while src[ptr] != "\"":
        result += src[ptr]
        ptr += 1
    return result

def parse_rule_output(src, ptr):
    # Get straight to the first argument
    ptr += len("tup.rule")
    # Get to parenthese
    ptr = get_to(src, ptr, "(")
    # Get to the closing parenthese
    ptr = get_to_block_finisher(src, ptr)
    # We are at closing parenthese of argument list
    # And the last argument is always output file
    # Let's get to closing "\"" of the output file name
    ptr = get_to(src, ptr, "\"", backwards = True)
    # Get into the string
    ptr -= 1
    # Then get to the beginning of the string
    ptr = get_to(src, ptr, "\"", backwards = True)
    # Now we can read the string
    return get_strnig(src, ptr)

def parse_required_compiler(src, ptr):
    # Get straignt to the first argument
    ptr += len("tup.getconfig")
    # Get to parenthese
    ptr = get_to(src, ptr, "(")
    # Get to start of the requirement string
    ptr = get_to(src, ptr, "\"")
    # Read the requirement string (like NO_FASM)
    requirement_string = get_strnig(src, ptr)
    if requirement_string.startswith("NO_"):
        return requirement_string[len("NO_"):].lower().replace("_", "-")
    else:
        return None

def parse_tupfile_outputs(file_name):
    outputs = []
    with open(file_name) as f:
        tupfile = f.read()
    rule_begin_index = tupfile.find("tup.rule(")
    while (rule_begin_index != -1):
        outputs.append(parse_rule_output(tupfile, rule_begin_index))
        # Find the next tup.rule call
        rule_begin_index = tupfile.find("tup.rule(", rule_begin_index + len("tup.rule("))
    return outputs

def parse_required_compilers(file_name):
    compilers = []
    with open(file_name) as f:
        tupfile = f.read()
    rule_begin_index = tupfile.find("tup.getconfig(")
    while (rule_begin_index != -1):
        required_compiler = parse_required_compiler(tupfile, rule_begin_index)
        if required_compiler is not None:
            compilers.append(required_compiler)
        # Find the next tup.getconfig call
        rule_begin_index = tupfile.find("tup.getconfig(", rule_begin_index + len("tup.getconfig"))
    return compilers
 
