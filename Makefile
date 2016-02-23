TOPDIR := $(realpath .)
SRCDIR := $(TOPDIR)/src
OPTS := -L$(SRCDIR) -I$(SRCDIR) -O3
LIBS := -lpwrusb

ifeq ($(strip $(PLATFORM)),)
	ifeq ($(OS),Windows_NT)
		PLATFORM := win32
	else
		PLATFORM := linux
	endif
endif

ifneq ($(PLATFORM),win32)
	LIBS += -ludev
endif


all: pwrusb

install: pwrusb

pwrusb:
	PLATFORM=$(PLATFORM) make -C $(SRCDIR)
	gcc $(OPTS) -o pwrusb_set pwrusb_set.c $(LIBS)

clean:
	make -C $(SRCDIR) clean
ifneq ($(PLATFORM),win32)
	rm -f pwrusb_set
else
	rm -f pwrusb_set.exe
endif

scratch: clean all

