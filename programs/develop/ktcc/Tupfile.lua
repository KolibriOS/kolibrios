local LIBS = {}

if tup.getconfig("NO_FASM") == "" then
    tup.append_table(LIBS, {
        tup.getcwd() .. "/libc.obj/source/crt0.o",
        tup.getcwd() .. "/lib/tiny/tiny.o",
    })
end

if tup.getconfig("NO_TCC") == "" then
    tup.append_table(LIBS, {
        tup.getcwd() .. "/lib/libcryptal/libcryptal.a",
        tup.getcwd() .. "/lib/libshell/libshell.a",
    })
end

if tup.getconfig("NO_FASM") == "" and tup.getconfig("NO_TCC") == "" then
    tup.append_table(LIBS, {
        tup.getcwd() .. "/libc.obj/source/libtcc1/libtcc1.a",
    })
end

tup.foreach_rule(LIBS, "cp %f %o", { tup.getcwd() .. "/bin/lib/%b", "<%b>" })

-- libsound.a lives in another subtree (contrib/), so reference its group
-- explicitly via extra_inputs to keep Tup ordering correct across trees.
if tup.getconfig("NO_FASM") == "" then
    tup.rule(
        {
            "../../../contrib/sdk/sources/sound/libsound.a",
            extra_inputs = { "../../../contrib/sdk/sources/sound/<libsound.a>" },
        },
        "cp %f %o",
        { tup.getcwd() .. "/bin/lib/libsound.a", "<libsound.a>" }
    )
end
