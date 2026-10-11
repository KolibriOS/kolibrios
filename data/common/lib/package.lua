tup.append_table(tmp_img_files, {
  -- {"LIB/ICONV.OBJ", tup.getcwd() .. "/iconv.obj"},
  {"LIB/KMENU.OBJ", tup.getcwd() .. "/kmenu.obj"},
  {"LIB/PIXLIB.OBJ", tup.getcwd() .. "/pixlib.obj"},
})

tup.append_table(tmp_extra_files, {
  {"kolibrios/lib/avcodec-56.dll", tup.getcwd() .. "/avcodec-56.dll"},
  {"kolibrios/lib/avdevice-56.dll", tup.getcwd() .. "/avdevice-56.dll"},
  {"kolibrios/lib/avformat-56.dll", tup.getcwd() .. "/avformat-56.dll"},
  {"kolibrios/lib/swscale-3.dll", tup.getcwd() .. "/swscale-3.dll"},
  {"kolibrios/lib/avutil-54.dll", tup.getcwd() .. "/avutil-54.dll"},
  {"kolibrios/lib/cairo2.dll", tup.getcwd() .. "/cairo2.dll"},
  {"kolibrios/lib/freetype.dll", tup.getcwd() .. "/freetype.dll"},
  {"kolibrios/lib/i965-video.dll", tup.getcwd() .. "/i965-video.dll"},
  {"kolibrios/lib/libdrm.dll", tup.getcwd() .. "/libdrm.dll"},
  {"kolibrios/lib/libegl.dll", tup.getcwd() .. "/libegl.dll"},
  {"kolibrios/lib/libeglut.dll", tup.getcwd() .. "/libeglut.dll"},
  {"kolibrios/lib/libGL.dll", tup.getcwd() .. "/libGL.dll"},
  {"kolibrios/lib/libjpeg.dll", tup.getcwd() .. "/libjpeg.dll"},
  {"kolibrios/lib/libpng16.dll", tup.getcwd() .. "/libpng16.dll"},
  {"kolibrios/lib/libva.dll", tup.getcwd() .. "/libva.dll"},
  {"kolibrios/lib/libz.dll", tup.getcwd() .. "/libz.dll"},
  {"kolibrios/lib/osmesa.dll", tup.getcwd() .. "/osmesa.dll"},
  {"kolibrios/lib/pixlib-gl.dll", tup.getcwd() .. "/pixlib-gl.dll"},
  {"kolibrios/lib/pixman-1.dll", tup.getcwd() .. "/pixman-1.dll"},
  {"kolibrios/lib/swresample-1.dll", tup.getcwd() .. "/swresample-1.dll"},
  {"kolibrios/lib/i915_dri.drv", tup.getcwd() .. "/i915_dri.drv"},
})
