#
# Tiny BASIC Interpreter and Compiler Project
# Makefile
# 
# Released as Public Domain by Damian Gareth Walker 2019
# Created: 18-Aug-2019
#

# Target
TARGET = tinybasic

# Paths and extensions
SRCDIR := src
INCDIR := inc
DOCDIR := doc
BASDIR := bas
BUILDDIR := obj
TARGETDIR := bin
INSTALLDIR := /usr/local
SRCEXT := c
HDREXT := h
OBJEXT := o
MANEXT := man
BASEXT := bas

# Compiler flags
CFLAGS := -Wall
INC := -I$(INCDIR) -I/usr/local/include

# Generate file lists
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))
SAMPLES := $(shell find $(BASDIR) -type f -name *.$(BASEXT))

# Default make
all: $(TARGETDIR)/$(TARGET)

$(TARGETDIR)/$(TARGET): $(OBJECTS)
	gcc -o $(TARGETDIR)/$(TARGET) $(OBJECTS)

$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	gcc $(CFLAGS) $(INC) -c -o $@ $<

# Cleanup
clean:
	rm -f $(BUILDDIR)/*.$(OBJEXT)
	rm -f $(TARGETDIR)/$(TARGET)

# Installation (Unix)
install: $(TARGETDIR)/$(TARGET) $(DOCDIR)/tinybasic.man $(SAMPLES)
	mkdir -p $(INSTALLDIR)/bin
	cp $(TARGETDIR)/$(TARGET) $(INSTALLDIR)/bin
	mkdir -p $(INSTALLDIR)/share/man/man1
	cp $(DOCDIR)/tinybasic.man $(INSTALLDIR)/share/man/man1/tinybasic.1
	mkdir -p $(INSTALLDIR)/share/doc/tinybasic/samples
	cp $(SAMPLES) $(INSTALLDIR)/share/doc/tinybasic/samples