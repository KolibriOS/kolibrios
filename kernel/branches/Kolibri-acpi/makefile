FASM=fasm
FLAGS=-m 65536
languages=en|ru|ge|et
drivers_src=com_mouse emu10k1x ensoniq fm801 infinity sis sound uart viasound vmode vt823\(x\)
skins_src=default

.PHONY: all kernel drivers skins clean

all: kernel drivers skins

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
	@mv bin/drivers/vmode.obj bin/drivers/vmode.mdr

skins:
	@echo "*** building skins ..."
	@mkdir -p bin/skins
	@cd skin; for f in $(skins_src); do \
	  echo "--- building 'bin/skins/$${f}.skn' ..."; \
	  $(FASM) $(FLAGS) $${f}.asm ../bin/skins/$${f}.skn || exit $?; \
	done

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
