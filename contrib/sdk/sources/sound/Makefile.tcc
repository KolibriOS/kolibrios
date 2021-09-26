AR      = ar
FASM    = fasm
OBJCOPY = objcopy

TARGET:= libsound

dirs := ./src

target_srcs:= $(foreach dir,$(dirs),$(wildcard $(dir)/*.asm))

target_objs:= $(subst .asm,.o,$(target_srcs))


all: $(TARGET).a

%.o: %.asm
	$(FASM) $< $@

	
$(TARGET).a: $(target_objs) Makefile
	
	$(AR) cvrs $@ $(target_objs)
	objcopy -O elf32-i386 --redefine-syms=symbols $@
	
clean:
	rm -f $(target_objs) $(TARGET).a
