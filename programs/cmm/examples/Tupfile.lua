if tup.getconfig("NO_CMM") ~= "" then return end
if tup.getconfig("LANG") == "ru"
then C_LANG = "LANG_RUS"
else C_LANG = "LANG_ENG" -- this includes default case without config
end
tup.rule("window.c", "c-- /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "window.com")
tup.rule("collections.c", "c-- /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "collections.com")
tup.rule("menu.c", "c-- /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "menu.com")
tup.rule("rgb.c", "c-- /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "rgb.com")
tup.rule("console.c", "c-- /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "console.com")
tup.rule("pigex.c", "c-- /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "pigex.com")
tup.rule("math.c", "c-- /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "math.com")
