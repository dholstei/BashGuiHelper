# 
#	BashGuiHelper - Use FLTK to prompt user for files, list selection, numeric input, date, etcetera
#	Programmer: Danny Holstein
#
LOG?=Install.log	#	Use Linux "logger" command to send logging info to this file
ifeq (null,$(shell if ls -l /dev/log  2>/dev/null > /dev/null ; then echo system; else echo null; fi))
	LOGGER?={ read LOGMSG && echo "[$@: `date`]" $$LOGMSG | tee -a $(LOG); }
else
	LOGGER?=logger --tag "[$@: `date`]" -s 2>&1 | tee -a $(LOG)
endif
CPP=g++
CPPFLAGS=$(DEBUG) -std=c++17 -fpermissive -Wno-write-strings
FLTKPREFIX:=/usr
FLTKBRANCH:=1.3
INCLUDES:=-I/usr/include/libxml2
INCLUDES:=$(INCLUDES) -I$(FLTKPREFIX)/include/FL
INCLUDES:=$(INCLUDES) -I/usr/include
INCLUDES:=$(INCLUDES) -I./
LDFLAGS=$(DEBUG)
LDLIBS:=-lxml2
ifeq ($(STATIC),)
	FLTKLIB:=$(FLTKPREFIX)/lib64/libfltk.so
	LDLIBS:=$(LDLIBS) $(FLTKLIB)
else
	FLTKLIB:=$(FLTKPREFIX)/lib64/libfltk.a
	LDLIBS:=$(LDLIBS) $(FLTKLIB) \
	-ldl -lX11 -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lXrender
endif
ifeq ($(DEBUG),)
	BINDIR:=release
else
	BINDIR:=debug
endif

all: $(BINDIR)/BashGuiHelper

fltk:	$(FLTKLIB)

$(FLTKLIB):
	@rm -rf repo/
	@if ./FLTKInstall.sh wget_fltk ; \
		then echo "--- FLTK download: Success ---" | $(LOGGER) ;\
		else echo "--- FLTK download: FAILURE! ---" | $(LOGGER) ; exit 1; fi
	@export PREFIX=$(FLTKPREFIX);\
	if ./FLTKInstall.sh config_fltk ; \
		then echo "--- FLTK CMAKE configure: Success ---" | $(LOGGER) ;\
		else echo "--- FLTK CMAKE configure: FAILURE! ---" | $(LOGGER) ; exit 1; fi
	@if ./FLTKInstall.sh build ; \
		then echo "--- FLTK build: Success ---" | $(LOGGER) ;\
		else echo "--- FLTK build: FAILURE! ---" | $(LOGGER) ; exit 1; fi
	@if sudo ./FLTKInstall.sh install ; \
		then echo "--- FLTK install: Success ---" | $(LOGGER) ;\
		else echo "--- FLTK install: FAILURE! ---" | $(LOGGER) ; exit 1; fi

OBJECTS=BashGuiHelper.o LibXML2.o

%.o:	%.cpp
	@if $(CPP) $(CPPFLAGS) $(INCLUDES) -c $^;\
		then echo "--- Compile $^: Success ---" | $(LOGGER) ;\
		else echo "--- Compile $^: FAILURE! ---" | $(LOGGER) ; exit 1; fi

$(BINDIR)/BashGuiHelper:	$(FLTKLIB) $(OBJECTS)
	@mkdir -p $(BINDIR)
	@if $(CPP) $(LDFLAGS) -o $@ $^ $(LDLIBS);\
		then echo "--- Build $@: Success ---" | $(LOGGER) ;\
		else echo "--- Build $@: FAILURE! ---" | $(LOGGER) ; exit 1; fi

test_tree:	$(BINDIR)/BashGuiHelper
	$< --type=tree --title="long title" --desc="long desc" --xml=./selection.xml

test_selection:	$(BINDIR)/BashGuiHelper
	$< --type=selection --title="long title" --desc="long desc" --directory=$$HOME/src --file_type="Image Files (*.png *.jpg *.bmp)" --items="Item 1\tItem 2\tItem 3 and a half"

clean:
	@if rm -fv release/BashGuiHelper debug/BashGuiHelper *.o && rm -rf repo/;\
		then echo "--- $@: Success ---" | $(LOGGER) ;\
		else echo "--- $@: FAILURE! ---" | $(LOGGER) ; exit 1; fi

clean-fltk:
	@if sudo rm -rf $(FLTKLIB);\
		then echo "--- $@: Success ---" | $(LOGGER) ;\
		else echo "--- $@: FAILURE! ---" | $(LOGGER) ; exit 1; fi