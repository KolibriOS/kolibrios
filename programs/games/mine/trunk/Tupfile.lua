if tup.getconfig("NO_CMM") ~= "" then return end
tup.rule("mine.c--", "c-- %f /meos" .. tup.getconfig("KPACK_CMD"), "mine")
