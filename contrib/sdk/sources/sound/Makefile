
AR = kos32-ar
FASM:= fasm

TARGET:= libsound

dirs := ./src

target_srcs:= $(foreach dir,$(dirs),$(wildcard $(dir)/*.asm))

target_objs:= $(subst .asm,.o,$(target_srcs))


all: $(TARGET).a

%.o: %.asm
	$(FASM) $< $@

$(TARGET).a: $(target_objs) Makefile   
	$(AR) cvrs $@ $(target_objs)
	mv -f $@ ../../lib
