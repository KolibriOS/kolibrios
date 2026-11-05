if tup.getconfig("NO_TCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_tcc.lua")

if tup.getconfig("LANG") == "ru_RU"
then C_LANG = "LANG_RUS"
elseif tup.getconfig("LANG") == "es_ES"
then C_LANG = "LANG_SPA"
else C_LANG = "LANG_ENG" -- this includes default case without config
end
CFLAGS = CFLAGS .. " -D" .. C_LANG

LIBS = "-lbox_lib -ldialog"
-- the formulas nest deep: the 4 KB tcc gives by default is not enough
LFLAGS = "-stack=65536"

link_tcc({
    "table.c",
    "sheet/cells.c", "sheet/changes.c", "sheet/refs.c", "sheet/texts.c", "sheet/undo.c",
    "formula/formula.c",
    "files/csv.c", "files/encoding.c", "files/file.c",
    "dialogs/help.c", "dialogs/look.c", "dialogs/menu.c",
    "window/commands.c", "window/edit.c", "window/grid.c", "window/input.c", "window/window.c",
}, "table")
