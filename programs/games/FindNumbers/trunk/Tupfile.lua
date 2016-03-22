if tup.getconfig("NO_CMM") ~= "" then return end
tup.rule("FindNumbers.c--", "c-- %f" .. tup.getconfig("KPACK_CMD"), "FindNumbers")
