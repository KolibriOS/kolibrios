tup.append_table(tmp_img_files, {
  {"MEDIA/AC97SND", tup.getcwd() .. "/ac97snd"},
})

tup.append_table(tmp_extra_files, {
  {"kolibrios/grafx2/fonts/", tup.getcwd() .. "/grafx2/fonts/*"},
  {"kolibrios/grafx2/scripts/", tup.getcwd() .. "/grafx2/scripts/libs/*"},
  {"kolibrios/grafx2/scripts/libs/", tup.getcwd() .. "/grafx2/scripts/*"},
  {"kolibrios/grafx2/skins/", tup.getcwd() .. "/grafx2/skins/*"},
  {"kolibrios/grafx2/", tup.getcwd() .. "/grafx2/*"},
  {"kolibrios/speech/", tup.getcwd() .. "/speech/*"},
  {"kolibrios/media/fplay", tup.getcwd() .. "/fplay"},
  {"kolibrios/media/fplay_run", tup.getcwd() .. "/fplay_run"},
  {"kolibrios/media/ImgF/ImgF", tup.getcwd() .. "/ImgF/ImgF"},
  {"kolibrios/media/ImgF/cEdg.obj", tup.getcwd() .. "/ImgF/cEdg.obj"},
  {"kolibrios/media/ImgF/dither.obj", tup.getcwd() .. "/ImgF/dither.obj"},
  {"kolibrios/media/ImgF/invSol.obj", tup.getcwd() .. "/ImgF/invSol.obj"},
  {"kolibrios/media/minimp3", tup.getcwd() .. "/minimp3"},
  {"kolibrios/media/vttf", tup.getcwd() .. "/vttf"},
})
