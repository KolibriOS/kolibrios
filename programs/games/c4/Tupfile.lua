if tup.getconfig("NO_NASM") ~= "" then return end
-- tup.rule is too unmannerly to %define
tup.definerule{
  command = "echo %%define lang '" .. ((tup.getconfig("LANG") == "") and "en_US" or tup.getconfig("LANG")) .. "'> %o",
  outputs = {"lang_nasm.inc"}
}
tup.rule({"c4.asm", extra_inputs = {"lang_nasm.inc"}}, "nasm -I" .. tup.getvariantdir() .. "/ -f bin -o %o %f " .. tup.getconfig("KPACK_CMD"), "c4")
