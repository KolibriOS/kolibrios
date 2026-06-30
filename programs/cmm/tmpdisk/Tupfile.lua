if tup.getconfig("NO_CMM") ~= "" then return end
if tup.getconfig("LANG") == "ru_RU"
then C_LANG = "LANG_RUS"
elseif tup.getconfig("LANG") == "es_ES"
then C_LANG = "LANG_SPA"
else C_LANG = "LANG_ENG" -- this includes default case without config
end
tup.rule("tmpdisk.c", "c-- /D=$(C_LANG) /OPATH=%o %f" .. tup.getconfig("KPACK_CMD"), "tmpdisk.com")
