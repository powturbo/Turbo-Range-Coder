# powturbo (c) Copyright 2013-2019
# Download or clone TurboPFor:
# git clone git://github.com/powturbo/Turbo-Range-Coder.git

# uncomment to include
#BWT=1
#BWTDIV=1
#NZ=1
#SH=1

CC ?= gcc
CXX ?= g++

#CC=powerpc64le-linux-gnu-gcc
#CL = $(CC)
#DEBUG=-DDEBUG -g
DEBUG=-DNDEBUG -s

PREFIX ?= /usr/local
DIRBIN ?= $(PREFIX)/bin
DIRINC ?= $(PREFIX)/include
DIRLIB ?= $(PREFIX)/lib

OPT=-fstrict-aliasing
ifeq (,$(findstring clang, $(CC)))
OPT+=-falign-loops
endif

#------- OS/ARCH -------------------
ifneq (,$(filter Windows%,$(OS)))
  OS := Windows
  CC=gcc
  CXX=g++
  ARCH=x86_64
  LDFLAGS+=-Wl,--stack,8194304
else
  OS := $(shell uname -s)
  ARCH := $(shell uname -m)

ifneq (,$(findstring aarch64,$(CC)))
  ARCH = aarch64
else ifneq (,$(findstring powerpc64le,$(CC)))
  ARCH = ppc64le
endif
endif

ifeq ($(ARCH),ppc64le)
  _SSE=-D__SSSE3__
  CFLAGS=-mcpu=power9 -mtune=power9 $(_SSE)
else ifeq ($(ARCH),aarch64)
  CFLAGS+=-march=armv8-a 
ifneq (,$(findstring clang, $(CC)))
  CFLAGS+=-march=armv8-a 
  OPT+=-fomit-frame-pointer
else
  CFLAGS+=-march=armv8-a 
endif
  SSE=-march=armv8-a
else ifeq ($(ARCH),$(filter $(ARCH),x86_64))
# set minimum arch sandy bridge SSE4.1 + AVX
  _SSE=-march=corei7-avx -mtune=corei7-avx 
# SSE+=-mno-avx -mno-aes
  _AVX2=-march=haswell
#  CFLAGS=$(SSE)
#  CFLAGS=$(AVX2)
endif

ifeq ($(AVX2),1)
CFLAGS=$(_AVX2)
else
CFLAGS=$(_SSE)
endif

CFLAGS+= $(DEBUG) $(OPT) -w -Wall
ifeq ($(PGO), 1)
CFLAGS+=-fprofile-generate 
LDFLAGS+=-lgcov
else ifeq ($(PGO), 2)
CFLAGS+=-fprofile-use 
endif

ifeq ($(OS),$(filter $(OS),Linux GNU/kFreeBSD GNU OpenBSD FreeBSD DragonFly NetBSD MSYS_NT Haiku))
#LDFLAGS+=-lrt
endif

ifeq ($(EXTRC), 1)
CFLAGS+=-DEXTRC
endif

ifeq ($(STATIC),1)
LDFLAGS+=-static
endif

ifeq ($(BWT), 1)

ifeq ($(BWTDIV), 1)
CFLAGS+=-DPROJECT_VERSION_FULL="20137" -DINLINE=inline -Ilibdivsufsort/include -Ilibdivsufsort/build/include 
LIBBWT =libdivsufsort/lib/sssort.o libdivsufsort/lib/utils.o libdivsufsort/lib/daware.o libdivsufsort/unbwt.o 
LIBBWT+=libdivsufsort/lib/divsufsort.o
CFLAGS+=-D_BWT -D_BWTDIV
else
CFLAGS+=-D_LIBBSC -ICSC/src/libcsc -DLIBBSC_SORT_TRANSFORM_SUPPORT 
#B=../../tb/
LIBBWT+=$(B)libbsc/libbsc/libbsc/libbsc.o $(B)libbsc/libbsc/coder/coder.o $(B)libbsc/libbsc/coder/qlfc/qlfc.o $(B)libbsc/libbsc/coder/qlfc/qlfc_model.o $(B)libbsc/libbsc/platform/platform.o $(B)libbsc/libbsc/filters/detectors.o \
	$(B)libbsc/libbsc/filters/preprocessing.o $(B)libbsc/libbsc/adler32/adler32.o $(B)libbsc/libbsc/bwt/bwt.o $(B)libbsc/libbsc/bwt/libsais/libsais.o $(B)libbsc/libbsc/st/st.o $(B)libbsc/libbsc/lzp/lzp.o
CFLAGS+=-D_BWT
endif

endif

ifeq ($(EXT), 1)
include ext.mk
endif

all: turborc

LIB=turborcs.o turborcss.o turborccdf.o 

ifeq ($(NZ), 1)
LIB+=turborcnz.o
CFLAGS+=-D_NZ
endif

ifeq ($(SH), 1)
LIB+=turborcsh.o
CFLAGS+=-D_SH
endif

#librc.a: $(LIB)
#	ar cr $@ $+

turborc.o: turborc.c
	$(CC) -O3 $(CFLAGS) -c turborc.c -o turborc.o

turborc: $(LIB) $(LIBBWT) turborc.o
	$(CC) $^ $(LDFLAGS) -o turborc

.c.o:
	$(CC) -O3 $(CFLAGS) $< -c -o $@

.cpp.o:
	$(CXX) -O3 $(MARCH) $(CXXFLAGS) $< -c -o $@ 

ifeq ($(OS),Windows_NT)
clean:
	del /S *.o
	del /S *~
else
clean:
	@find . -type f -name "*\.o" -delete -or -name "*\~" -delete -or -name "core" -delete -or -name "turborc" -delete -or -name "librc.a" -delete
endif

