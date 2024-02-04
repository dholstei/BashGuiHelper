# 
#	BashGuiHelper - Use FLTK to prompt user for files, list selection, numeric input, date, etcetera
#	Programmer: Danny Holstein
#
CPP=g++
CPPFLAGS=$(DEBUG) -std=c++17 -fpermissive -Wno-write-strings
FLTKPREFIX:=/usr
INCLUDES:=-I/usr/include/libxml2
INCLUDES:=$(INCLUDES) -I$(FLTKPREFIX)/include/FL
INCLUDES:=$(INCLUDES) -I/usr/include
INCLUDES:=$(INCLUDES) -I./
LDFLAGS=$(DEBUG)
LDLIBS:=-lxml2
ifeq ($(STATIC),)
	LDLIBS:=$(LDLIBS) $(FLTKPREFIX)/lib64/libfltk.so
else
	LDLIBS:=$(LDLIBS) $(FLTKPREFIX)/lib64/libfltk.a \
	-ldl -lX11 -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lXrender
endif
ifeq ($(DEBUG),)
	BINDIR=release
else
	BINDIR=debug
endif

all: $(BINDIR)/BashGuiHelper

OBJECTS=BashGuiHelper.o LibXML2.o

%.o:	%.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $^

$(BINDIR)/BashGuiHelper:	$(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CPP) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -fv release/BashGuiHelper debug/BashGuiHelper *.o