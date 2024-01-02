#
#   Makefile 
#   Jinwook Shin
#   Mayura Worlikar
#   ...
#
#   Tab Size is 4 (in vim, :set tabstop=4).
#
.SUFFIXES   =.cpp.o

CPP         =g++
CFLAGS      =-ggdb      # for debug mode
#CFLAGS     =-Wall -ggdb        # for debug mode
#CFLAGS     =-O2 -DNDEBUG       # for release mode 
#CFLAGS     =-DNDEBUG       # for release mode 
LDFLAGS     =
INCS        =-I.
AR          = ar
RANLIB      = ranlib

TARGET      =dbms
LIB         =

SRCS1       =tokenizer.cpp main.cpp theta.cpp
OBJS1       =tokenizer.o main.o theta.o
HDRS1       =BufferManager.h Gvariables.h Parser.h QEE.h tokenizer.h FA.h theta.h

all: $(TARGET)

$(TARGET): $(OBJS1)
	$(CPP) -o $@ $(OBJS1) $(INC) ${LDFLAGS} 

$(OBJS1): $(SRCS1) $(HDRS1)
	$(CPP) $(INCS) $(CFLAGS) -c $(SRCS1)

#$(LIB): $(OBJS2)
#   $(AR) rcu $@ $^
#   -$(RANLIB) $@

.PHONY: format
format:
	clang-format -i $(SRCS1) $(HDRS1) 

clean:
	rm -f $(OBJS1) $(TARGET)