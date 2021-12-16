if tup.getconfig("NO_CMM") ~= "" then return end
if tup.getconfig("LANG") == "ru"
then C_LANG = "LANG_RUS"
else C_LANG = "LANG_ENG" -- this includes default case without config
end
tup.rule("acpi_install.c", "c-- /D=AUTOBUILD /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "acpi_install.com")
tup.rule("easyshot.c", "c-- /D=AUTOBUILD /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "easyshot.com")
tup.rule("install.c", "c-- /D=AUTOBUILD /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "install.com")
tup.rule("mblocks.c", "c-- /D=AUTOBUILD /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "mblocks.com")
tup.rule("notify.c", "c-- /D=AUTOBUILD /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "notify.com")
tup.rule("osupdate.c", "c-- /D=AUTOBUILD /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "osupdate.com")
tup.rule("pipet.c", "c-- /D=AUTOBUILD /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "pipet.com")
tup.rule("software_widget.c", "c-- /D=AUTOBUILD /D=$(C_LANG) %f" .. tup.getconfig("KPACK_CMD"), "software_widget.com")
tup.rule("reshare.c", "c-- %f" .. tup.getconfig("KPACK_CMD"), "reshare.com")
tup.rule("kfm2.c", "c-- %f" .. tup.getconfig("KPACK_CMD"), "kfm2.com")

