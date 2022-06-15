# DGen/SDL v1.31+
# Automakefile for Cyclone 68000

CC = @CC_FOR_BUILD@
CXX = @CXX_FOR_BUILD@
@CPPFLAGS_B@ = @CPPFLAGS_FOR_BUILD@ -I$(DIR)/Disa
@CFLAGS_B@ = @CFLAGS_FOR_BUILD@
@CXXFLAGS_B@ = @CXXFLAGS_FOR_BUILD@
@LDFLAGS_B@ = @LDFLAGS_FOR_BUILD@
OBJEXT = @OBJEXT_FOR_BUILD@
EXEEXT = @EXEEXT_FOR_BUILD@

CYCLONE = cyclone$(EXEEXT)
DIR = $(top_srcdir)/cyclone

bin_PROGRAMS = cyclone

cyclone_SOURCES =	\
	app.h		\
	config.h	\
	Cyclone.h	\
	Ea.cpp		\
	Main.cpp	\
	OpAny.cpp	\
	OpArith.cpp	\
	OpBranch.cpp	\
	OpLogic.cpp	\
	OpMove.cpp	\
	Disa/Disa.c	\
	Disa/Disa.h

BUILT_SOURCES = Cyclone.s

$(BUILT_SOURCES): $(CYCLONE)
	./$(CYCLONE)

noinst_LIBRARIES = libcyclonecpu.a
libcyclonecpu_a_SOURCES = $(BUILT_SOURCES)
libcyclonecpu_a_DEPENDENCIES = $(CYCLONE)

EXTRA_DIST = Cyclone.txt

DISTCLEANFILES = $(BUILT_SOURCES) $(CYCLONE)
