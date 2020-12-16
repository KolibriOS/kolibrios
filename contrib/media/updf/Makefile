# GNU Makefile

OUT := build
GEN := generated

# --- Variables, Commands, etc... ---

default: all
LDFLAGS = -Tinclude/scripts/menuetos_app_v01.ld -nostdlib -L include/lib
CFLAGS += -Ifitz -Ipdf -Iscripts  -fno-stack-protector -nostdinc -fno-builtin -m32 -I include -fno-pic -w
LIBS += -lfreetype2 -lpng -ljbig2dec -ljpeg -lopenjpeg -lz -lm -lc

THIRD_LIBS := $(FREETYPE_LIB) $(JBIG2DEC_LIB) $(JPEG_LIB) $(OPENJPEG_LIB) $(ZLIB_LIB)

ifeq "$(verbose)" ""
QUIET_AR = @ echo ' ' ' ' AR $@ ;
QUIET_CC = @ echo ' ' ' ' CC $@ ;
QUIET_GEN = @ echo ' ' ' ' GEN $@ ;
QUIET_LINK = @ echo ' ' ' ' LINK $@ ;
QUIET_MKDIR = @ echo ' ' ' ' MKDIR $@ ;
endif

CC_CMD = $(QUIET_CC) $(CC) $(CFLAGS) -o $@ -c $<
AR_CMD = $(QUIET_AR) $(AR) cru $@ $^
LINK_CMD = $(QUIET_LINK) ld $(LDFLAGS) -o $@ include/stub/crt0.o $^ build/snprintf.o $(LIBS)
MKDIR_CMD = $(QUIET_MKDIR) mkdir -p $@

# --- Rules ---

$(OUT) $(GEN) :
	$(MKDIR_CMD)

$(OUT)/%.a :
	$(AR_CMD)

$(OUT)/% : $(OUT)/%.o
	$(LINK_CMD)

$(OUT)/%.o : fitz/%.c fitz/fitz.h | $(OUT)
	$(CC_CMD)
$(OUT)/%.o : draw/%.c fitz/fitz.h | $(OUT)
	$(CC_CMD)
$(OUT)/%.o : pdf/%.c fitz/fitz.h pdf/mupdf.h | $(OUT)
	$(CC_CMD)
$(OUT)/%.o : apps/%.c fitz/fitz.h pdf/mupdf.h | $(OUT)
	$(CC_CMD)
$(OUT)/%.o : scripts/%.c | $(OUT)
	$(CC_CMD)

# --- Fitz and MuPDF libraries ---

FITZ_LIB := $(OUT)/libfitz.a
MUPDF_LIB := $(OUT)/libmupdf.a

FITZ_SRC := $(notdir $(wildcard fitz/*.c draw/*.c))
MUPDF_SRC := $(notdir $(wildcard pdf/*.c))

$(FITZ_LIB) : $(addprefix $(OUT)/, $(FITZ_SRC:%.c=%.o))
$(MUPDF_LIB) : $(addprefix $(OUT)/, $(MUPDF_SRC:%.c=%.o))

libs: $(MUPDF_LIB) $(FITZ_LIB) $(THIRD_LIBS)

# --- Generated CMAP and FONT files ---

CMAPDUMP := scripts/cmapdump

CMAP_CNS_SRC := $(wildcard cmaps/cns/*)
CMAP_GB_SRC := $(wildcard cmaps/gb/*)
CMAP_JAPAN_SRC := $(wildcard cmaps/japan/*)
CMAP_KOREA_SRC := $(wildcard cmaps/korea/*)
FONT_BASE14_SRC := $(wildcard fonts/*.cff)
FONT_DROID_SRC := fonts/droid/DroidSans.ttf fonts/droid/DroidSansMono.ttf
FONT_CJK_SRC := fonts/droid/DroidSansFallback.ttf

$(GEN)/cmap_cns.h : $(CMAP_CNS_SRC)
	$(QUIET_GEN) ./$(CMAPDUMP) $@ $(CMAP_CNS_SRC)
$(GEN)/cmap_gb.h : $(CMAP_GB_SRC)
	$(QUIET_GEN) ./$(CMAPDUMP) $@ $(CMAP_GB_SRC)
$(GEN)/cmap_japan.h : $(CMAP_JAPAN_SRC)
	$(QUIET_GEN) ./$(CMAPDUMP) $@ $(CMAP_JAPAN_SRC)
$(GEN)/cmap_korea.h : $(CMAP_KOREA_SRC)
	$(QUIET_GEN) ./$(CMAPDUMP) $@ $(CMAP_KOREA_SRC)

$(GEN)/font_base14.h : $(FONT_BASE14_SRC)
	$(QUIET_GEN) ./$(FONTDUMP) $@ $(FONT_BASE14_SRC)
$(GEN)/font_droid.h : $(FONT_DROID_SRC)
	$(QUIET_GEN) ./$(FONTDUMP) $@ $(FONT_DROID_SRC)
$(GEN)/font_cjk.h : $(FONT_CJK_SRC)
	$(QUIET_GEN) ./$(FONTDUMP) $@ $(FONT_CJK_SRC)

CMAP_HDR := $(addprefix $(GEN)/, cmap_cns.h cmap_gb.h cmap_japan.h cmap_korea.h)
FONT_HDR := $(GEN)/font_base14.h $(GEN)/font_droid.h $(GEN)/font_cjk.h

ifeq "$(CROSSCOMPILE)" ""
$(CMAP_HDR) : $(CMAPDUMP) | $(GEN)
$(FONT_HDR) : $(FONTDUMP) | $(GEN)
endif

generate: $(CMAP_HDR) $(FONT_HDR)

$(OUT)/pdf_cmap_table.o : $(CMAP_HDR)
$(OUT)/pdf_fontfile.o : $(FONT_HDR)
$(OUT)/cmapdump.o : pdf/pdf_cmap.c pdf/pdf_cmap_parse.c

# --- Tools and Apps ---

MUPDF := $(OUT)/mupdf
$(MUPDF) : $(MUPDF_LIB) $(FITZ_LIB) $(THIRD_LIBS)
ifeq "$(NOX11)" ""
$(MUPDF) : $(addprefix $(OUT)/, kos_main.o pdfapp.o)
	$(LINK_CMD) 
endif

all: $(THIRD_LIBS) $(FITZ_LIB) $(PDF_APPS) $(MUPDF)
