# 
#	BashQtHelp - Use Qt to prompt user for files, list selection, numeric input, date, etcetera
#	Programmer: Danny Holstein
#
CPP=g++
CPPFLAGS=$(DEBUG) -std=c++17 -fpermissive -I/usr/include/qt5/ -I/usr/include/libxml2/
LDFLAGS=$(DEBUG)
LDLIBS= -lQt5Core -lQt5Gui -lQt5Widgets -lxml2

ifeq ($(DEBUG),)
	BINDIR=release
else
	BINDIR=debug
endif

all: $(BINDIR)/BashQtHelper

OBJECTS=BashQtHelper.o LibXML2.o

%.o:	%.cpp LibXML2.h
	$(CPP) $(CPPFLAGS) -c $^

$(BINDIR)/BashQtHelper:	$(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CPP) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BINDIR)/slot:	slot.cpp
	@mkdir -p $(BINDIR)
	$(CPP) $(CPPFLAGS) -c $^
	$(CPP) $(LDFLAGS) -o $@ $(@F).o $(LDLIBS)

clean:
	rm -fv release/BashQtHelper debug/BashQtHelper *.o