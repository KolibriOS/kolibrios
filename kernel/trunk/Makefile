FASM=fasm
FLAGS=-m 262144
languages=en_US|ru_RU|de_DE|et_EE|es_ES

.PHONY: all kernel bootloader clean

all: kernel bootloader

kernel: check_lang
  @echo "*** building kernel with language '$(lang)' ..."
  @mkdir -p bin
  @echo "lang fix $(lang)" > lang.inc
  @echo "--- building 'bin/kernel.mnt' ..."
  @$(FASM) $(FLAGS) kernel.asm bin/kernel.mnt
  @$(FASM) $(FLAGS) kernel.asm -dextended_primary_loader=1 bin/kernel.bin
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