if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("uefi4kos.asm", "fasm -dUEFI=1 -dextended_primary_loader=1 %f %o", "kolibri.efi")
