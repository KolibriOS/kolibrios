FASM=fasm
FLAGS=-m 65536
languages=en|ru|ge|et|sp
drivers_src=com_mouse emu10k1x fm801 infinity sis sound vt823x

.PHONY: all kernel drivers bootloader clean

all: kernel drivers bootloader

kernel: check_lang
	@echo "*** building kernel with language '$(lang)' ..."
	@mkdir -p bin
	@echo "lang fix $(lang)" > lang.inc
	@echo "--- building 'bin/kernel.mnt' ..."
	@$(FASM) $(FLAGS) kernel.asm bin/kernel.mnt
	@rm -f lang.inc

drivers:
	@echo "*** building drivers ..."
	@mkdir -p bin/drivers
	@cd drivers; for f in $(drivers_src); do \
	  echo "--- building 'bin/drivers/$${f}.obj' ..."; \
	  $(FASM) $(FLAGS) "$${f}.asm" "../bin/drivers/$${f}.obj" || exit $?; \
	done

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
