if tup.getconfig("NO_FASM") ~= "" then return end
if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../../" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")

CFLAGS = " -c -nostdinc -DGNUC -D_BUILD_LIBC -Os -fno-common -fno-builtin -fno-leading-underscore -fno-pie"
INCLUDES = " -I../include"

OBJS = {"math/tan.obj", "math/sqrt.obj"}

for _, OBJ in pairs(OBJS) do
    tup.rule(string.gsub(OBJ, ".obj", ".asm"), "fasm %f %o ", OBJ)
end

tup.rule("libc.c", "kos32-gcc" .. CFLAGS .. INCLUDES .. " -o %o %f ", "libc_tmp.obj")

table.insert(OBJS, "libc_tmp.obj");
tup.rule(OBJS, "clink -o %o %f" .. " && kos32-strip %o --strip-unneeded " .. tup.getconfig("KPACK_CMD"), "libc.obj");
