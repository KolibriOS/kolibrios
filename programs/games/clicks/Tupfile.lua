if tup.getconfig("NO_CMM") ~= "" then return end
if tup.getconfig("LANG") == "ru_RU"
then C_LANG = "LANG_RUS"
else C_LANG = "LANG_ENG" -- this includes default case without config
end
tup.rule("clicks.c", "c-- /D=AUTOBUILD /D=$(C_LANG) /opath=%o %f" .. tup.getconfig("KPACK_CMD"), "clicks.com")
