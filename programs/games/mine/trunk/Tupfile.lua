if tup.getconfig("NO_CMM") ~= "" then return end
tup.rule("mine.c--", "c-- %f /meos /opath=%o" .. tup.getconfig("KPACK_CMD"), "mine")
