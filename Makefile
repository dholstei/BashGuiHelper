# 
#	BashGuiHelper - Use FLTK to prompt user for files, list selection, numeric input, date, etcetera
#	Programmer: Danny Holstein
#
CPP=g++
CPPFLAGS=$(DEBUG) -std=c++17
INCLUDES:=-I/usr/include/libxml2
INCLUDES:=$(INCLUDES) -I/usr/include/FL
LDFLAGS=$(DEBUG)
LDLIBS:=-lxml2
LDLIBS:=$(LDLIBS) -lfltk
ifeq ($(DEBUG),)
	BINDIR=release
else
	BINDIR=debug
endif

all: $(BINDIR)/BashGuiHelper

OBJECTS=BashGuiHelper.o LibXML2.o

%.o:	%.cpp LibXML2.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $^

$(BINDIR)/BashGuiHelper:	$(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CPP) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BINDIR)/slot:	slot.cpp
	@mkdir -p $(BINDIR)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $^
	$(CPP) $(LDFLAGS) -o $@ $(@F).o $(LDLIBS)

clean:
	rm -fv release/BashGuiHelper debug/BashGuiHelper *.o