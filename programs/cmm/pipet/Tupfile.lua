if tup.getconfig("NO_CMM") ~= "" then return end
if tup.getconfig("LANG") == "ru"
then C_LANG = "LANG_RUS"
else C_LANG = "LANG_ENG" -- this includes default case without config
end
tup.rule("pipet.c", "c-- /D=AUTOBUILD /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "pipet.com")
