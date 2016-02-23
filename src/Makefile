TOPDIR := $(realpath .)
OPTS := -O3
INCLUDES :=
LIBNAME := libpwrusb
VERSION := 1.0.0

ifneq ($(PLATFORM),win32)
	OPTS += -fpic -fvisibility=hidden -DBUILD_LIBPWRUSB -DHAVE_VISIBILITY
endif


all: pwrusb

pwrusb_generic.o:
	gcc -o pwrusb_generic.o $(OPTS) $(INCLUDES) -c pwrusb_generic.c

pwrusb_linux.o:
	gcc -o pwrusb_linux.o $(OPTS) $(INCLUDES) -c pwrusb_linux.c

pwrusb_win32.o:
	gcc -o pwrusb_win32.o $(OPTS) $(INCLUDES) -c pwrusb_win32.c

pwrusb: pwrusb_generic.o pwrusb_$(PLATFORM).o
	ar rcs libpwrusb.a pwrusb_generic.o pwrusb_$(PLATFORM).o
ifneq ($(PLATFORM),win32)
	gcc -shared -Wl,-soname,$(LIBNAME).so -o $(LIBNAME).so.$(VERSION) pwrusb_generic.o pwrusb_$(PLATFORM).o
	ldconfig -n $(TOPDIR)
else
	gcc -shared -Wl,--out-implib,$(LIBNAME).dll.a -o $(LIBNAME).dll pwrusb_generic.o pwrusb_$(PLATFORM).o
endif

clean:
	rm -f *.a *.o
ifneq ($(PLATFORM),win32)
	rm -f *.so *.so.*
else
	rm -f *.dll *.dll.*
endif

scratch: clean all

