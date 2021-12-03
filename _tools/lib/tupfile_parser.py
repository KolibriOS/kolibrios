# Copyright Magomed Kostoev
# Published under MIT license

class Rule:
    pass

def get_config(config_name):
    if config_name == "KPACK_CMD":
        # Just never pack the file for now
        return ""
    else:
        print(f"Unknown config name: {config_name}")
        exit(-1)

def skip_whitespaces(src, ptr):
    while len(src) > ptr and src[ptr] == " ":
        ptr += 1
    return ptr

# Returns True if @src has @string starting from @ptr index
def match_string(src, ptr, string):
    if len(src) <= ptr + len(string):
        return False
    for i in range(len(string)):
        if src[ptr + i] != string[i]:
            return False
    return True

def parse_tup_getconfig(src, ptr):
    # Skip get straight to the argument
    ptr += len("tup.getconfig(")
    ptr = skip_whitespaces(src, ptr)
    if src[ptr] != "\"":
        print("Expected \"config name\" as tup.getconfig parameter")
        exit()
    (config_name, ptr) = parse_string(src, ptr)
    ptr = skip_whitespaces(src, ptr)
    # Skip closing parenthese of the tup.getconfig call
    assert(src[ptr] == ")")
    ptr += 1
    return (get_config(config_name), ptr)

def parse_string(src, ptr):
    ptr += 1
    string = ""
    while src[ptr] != "\"":
        string += src[ptr]
        ptr += 1
    # Skip the closing "\""
    ptr += 1
    ptr = skip_whitespaces(src, ptr)
    # Check if we have concatination here
    if match_string(src, ptr, ".."):
        # Skip the ".."
        ptr += 2
        # The expression parsing should result in a string
        (string_to_add, ptr) = parse_expression(src, ptr)
        # Concat our string to the resulting string
        string += string_to_add
    return (string, ptr)

def parse_expression(src, ptr):
    ptr = skip_whitespaces(src, ptr)
    result = "WAT?!"
    if src[ptr] == "\"":
        (result, ptr) = parse_string(src, ptr)
    elif match_string(src, ptr, "tup.getconfig("):
        (result, ptr) = parse_tup_getconfig(src, ptr)
    else:
        print(f"Can't handle anything starting with '{src[ptr]}'")
        exit(-1)
    ptr = skip_whitespaces(src, ptr)
    return (result, ptr)

def expect_comma(src, ptr):
    comma_skept = False
    ptr = skip_whitespaces(src, ptr)
    if src[ptr] == ",":
        ptr += 1
        return (True, ptr)
    else:
        return (False, ptr)

def parse_arguments(src, ptr):
    result = []
    # Parse first argument
    (argument, ptr) = parse_expression(src, ptr)
    result.append(argument)
    (comma_encoutered, ptr) = expect_comma(src, ptr)
    # Parse the second argument if it's there
    if comma_encoutered:
        (argument, ptr) = parse_expression(src, ptr)
        result.append(argument)
        (comma_encoutered, ptr) = expect_comma(src, ptr)
    # Parse third argument if it's there
    if comma_encoutered:
        (argument, ptr) = parse_expression(src, ptr)
        result.append(argument)
    return result

def parse_rule(src, ptr):
    # Get straight to the first argument
    ptr += len("tup.rule(")
    # Parse the arguments
    args = parse_arguments(src, ptr)
    # Build the rule object
    result = Rule()
    if len(args) == 3:
        result.input = args[0]
        result.command = args[1]
        result.output = args[2]
        # Replace %f with input file in rule's command
        if type(result.input == str):
            result.command = result.command.replace("%f", result.input)
        else:
            print("Command building with non-string tup.rule's first argument"
                + " isn't implemented")
            exit()
        # Replace %o with output file in rule's command
        if type(result.output == str):
            result.command = result.command.replace("%o", result.output)
        else:
            print("Command building with non-string tup.rule's first argument"
                + " isn't implemented")
            exit()
    elif len(args) == 2:
        result.input = []
        result.command = args[0]
        result.output = args[1]
    else:
        print(f"tup.rule can only take 2 or 3 arguments, not {len(args)}")
        exit(-1)
    # Unify the API - return arrays as input and output
    if type(result.input) == str:
        result.input = [ result.input ]
    else:
        assert(type(result.input) == list)
    if type(result.output) == str:
        result.output = [ result.output ]
    else:
        assert(type(result.output) == list)
    return result

def parse(file_name):
    rules = []
    with open(file_name) as f:
        tupfile = f.read()
    rule_begin_index = tupfile.find("tup.rule(")
    while (rule_begin_index != -1):
        rules.append(parse_rule(tupfile, rule_begin_index))
        # Find the next tup.rule call
        rule_begin_index = tupfile.find("tup.rule(", rule_begin_index + len("tup.rule("))
    return rules
