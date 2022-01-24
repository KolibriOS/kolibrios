if tup.getconfig("NO_FASM") ~= "" then return end
if tup.getconfig("NO_TCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../../../"
end
tup.include(HELPERDIR .. "/use_tcc.lua")

CFLAGS = CFLAGS .. " -I../../include"
LFLAGS = "-stack=10000"

tup.rule("load_mbedtls.asm", "fasm %f %o ", "load_mbedtls.o")
link_tcc({"ssl_client1.c", "load_mbedtls.o"}, "ssl_client1")
