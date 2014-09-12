if tup.getconfig("TUP_PLATFORM") == "win32"
-- on win32 '#' is not a special character, but backslash and quotes would be printed as is
then tup.rule('echo #define NEEDS_UNDERSCORES > %o', {"config.h"})
-- on unix '#' should be escaped
else tup.rule('echo "#define NEEDS_UNDERSCORES" > %o', {"config.h"})
end
