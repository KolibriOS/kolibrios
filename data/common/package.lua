tup.include("./settings/package.lua")

tup.append_table(tmp_img_files, {
  {"ALLGAMES",                   "common/allgames"},
  {"HOME.PNG",                   "common/wallpapers/T_Home.png"},
  {"ICONS32.PNG",                "common/icons32.png"},
  {"ICONS18.PNG",                "common/icons18.png"},
  {"INDEX.HTM",                  "common/index_htm"},
  {"KUZKINA.MID",                "common/kuzkina.mid"},
  {"SINE.MP3",                   "common/sine.mp3"},
  {"NOTIFY3.PNG",                "common/notify3.png"},

  {"3D/HOUSE.3DS",               "common/3d/house.3ds"},

  {"File Managers/ICON2EXT.INI", "common/File Managers/icon2ext.ini"},

  {"FONTS/TAHOMA.KF",            "common/fonts/tahoma.kf"},

  -- {"LIB/ICONV.OBJ",           "common/lib/iconv.obj"},
  {"LIB/KMENU.OBJ",              "common/lib/kmenu.obj"},
  {"LIB/PIXLIB.OBJ",             "common/lib/pixlib.obj"},

  {"MEDIA/AC97SND",              "common/media/ac97snd"},

  {"NETWORK/FTPD.INI",           "common/network/ftpd.ini"},
  {"NETWORK/KNMAP",              "common/network/knmap"},
  {"NETWORK/USERS.INI",          "common/network/users.ini"},
})

if build_type ~= "ru_RU" then
  tup.append_table(tmp_img_files, {
    {"File Managers/KFAR.INI", "common/File Managers/kfar.ini"},
    {"GAMES/DESCENT",          "common/games/descent"},
  })
end
