if tup.getconfig('NO_GCC') ~= "" then return end
CFLAGS = "-Os -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -fno-ident -fomit-frame-pointer -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -march=pentium-mmx"
OBJS = tup.foreach_rule({"*.cpp", extra_inputs = {"../../config.h"}},
  'kos32-gcc -c -I../../include -D__DEV_CONFIG_H=\\"../../config.h\\" ' .. CFLAGS .. " -o %o %f",
  "%B.o")
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../../lib/libcpp.a", "../../<libcpp>"})
