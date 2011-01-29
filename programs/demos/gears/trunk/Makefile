OUTFILE = gears

SRCS = main.cpp fps.cpp 

OBJS = $(SRCS:.cpp=.o)

CPPFLAGS = -I$(TINYGL)\include -O2 

LIBR = TinyGL  m
LIBS = $(addprefix -l,$(LIBR)) -L$(TINYGL)\lib

all: $(OUTFILE)
	objcopy $(OUTFILE) -O binary

include $(MENUETDEV)/makefiles/Makefile_for_cpp_program
