if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("uefi64kos.asm", "fasm -dUEFI=1 -dextended_primary_loader=1 %f %o", "bootx64.efi")
tup.rule("uefi32kos.asm", "fasm -dUEFI=1 -dextended_primary_loader=1 %f %o", "bootia32.efi")
