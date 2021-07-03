FASM=fasm
FLAGS=-m 65536
languages=en|ru|ge|et|sp

.PHONY: all kernel bootloader clean

all: kernel bootloader bootbios

kernel: check_lang bootbios
	@echo "*** building kernel with language '$(lang)' ..."
	@mkdir -p bin
	@echo "lang fix $(lang)" > lang.inc
	@echo "--- building 'bin/kernel.mnt' ..."
	@$(FASM) $(FLAGS) kernel.asm bin/kernel.mnt
	@$(FASM) $(FLAGS) -dUEFI=1 kernel.asm bin/kernel.bin
	@rm -f lang.inc

bootbios: check_lang
	@echo "*** building bootbios.bin with language '$(lang)' ..."
	@mkdir -p bin
	@echo "lang fix $(lang)" > lang.inc
	@echo "--- building 'bootbios.bin' ..."
	@$(FASM) $(FLAGS) bootbios.asm bootbios.bin
	@rm -f lang.inc

bootloader: check_lang
	@echo "*** building bootloader with language '$(lang)' ..."
	@mkdir -p bin
	@echo "lang fix $(lang)" > lang.inc
	@echo "--- building 'bin/boot_fat12.bin' ..."
	@$(FASM) $(FLAGS) bootloader/boot_fat12.asm bin/boot_fat12.bin
	@rm -f lang.inc
		

check_lang:
	@case "$(lang)" in \
	  $(languages)) \
	    ;; \
	  *) \
	    echo "*** error: language is incorrect or not specified"; \
	    exit 1; \
	    ;; \
	esac

clean:
	rm -rf bin
	rm -f lang.inc
