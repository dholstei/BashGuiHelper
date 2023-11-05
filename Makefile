# 
#	BashQtHelp - Use Qt to prompt user for files, list selection, numeric input, date, etcetera
#	Programmer: Danny Holstein
#

CPPFLAGS=-g -std=c++17 -I/usr/include/qt5/
LDFLAGS=-g
LDLIBS= -lQt5Core -lQt5Gui -lQt5Widgets

all: BashQtHelper

BashQtHelper.o:	BashQtHelper.cpp
	g++ $(CPPFLAGS) -c $^

BashQtHelper: BashQtHelper.o
	g++ $(LDFLAGS) -o $@ $^ $(LDLIBS)
