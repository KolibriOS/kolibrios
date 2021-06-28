if tup.getconfig("NO_CMM") ~= "" then return end
if tup.getconfig("LANG") == "ru"
then C_LANG = "LANG_RUS"
else C_LANG = "LANG_ENG" -- this includes default case without config
end
tup.rule("tmpdisk.c", "c-- /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "tmpdisk.com")
