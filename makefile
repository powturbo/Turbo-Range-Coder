# powturbo (c) Copyright 2013-2023
# Download or clone TurboRC:
# git clone git://github.com/powturbo/Turbo-Range-Coder.git

# fsm predictor
#SF=1
# include BWT 
BWT=1
#BWTDIV=1
LIBSAIS16=1
#V8
#BWTSATAN=1
ANS=1
#EXT=1
#NOCOMP=1
#AVX2=1
#TURBORLE=1
TRANSPOSE=1
#----------------------------------------------
CC ?= gcc
CXX ?= g++
#CC=clang
#CXX=clang++

ifeq ($(PGO), 1)
CFLAGS=-fprofile-generate 
LDFLAGS+=-lgcov
else
#CFLAGS=-fprofile-use 
endif

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
#  CC=gcc
#  CXX=g++
  ARCH=x86_64
  LDFLAGS+=-Wl,--stack,33554432
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
  MARCH=-mcpu=power9 -mtune=power9 $(_SSE)
else ifeq ($(ARCH),aarch64)
  MARCH+=-march=armv8-a 
ifneq (,$(findstring clang, $(CC)))
  MARCH+=-march=armv8-a 
  OPT+=-fomit-frame-pointer
else
  MARCH+=-march=armv8-a 
endif
  _SSE=-march=armv8-a
else ifeq ($(ARCH),$(filter $(ARCH),x86_64))
  LDFLAG+=-lm
# set minimum arch sandy bridge SSE4.1 + AVX
  _SSE=-march=corei7-avx -mtune=corei7-avx 
# SSE+=-mno-avx -mno-aes
  _AVX2=-march=haswell
#  CFLAGS=$(SSE)
#  CFLAGS=$(AVX2)
endif

ifeq ($(AVX2),1)
MARCH=$(_AVX2) 
else
MARCH=$(_SSE) 
endif

CFLAGS+= $(DEBUG) $(OPT) -w -Wall -pedantic
ifeq ($(PGO), 1)
CFLAGS+=-fprofile-generate 
LDFLAGS+=-lgcov
else ifeq ($(PGO), 2)
CFLAGS+=-fprofile-use 
endif

ifeq ($(OS),$(filter $(OS),Darwin Linux GNU/kFreeBSD GNU OpenBSD FreeBSD DragonFly NetBSD MSYS_NT Haiku))
#LDFLAGS+=-lrt
LDFLAGS+=-lm 
#-Wl,--stack_size -Wl,20971520
endif

ifeq ($(EXTRC), 1)
CFLAGS+=-DEXTRC
endif

ifeq ($(STATIC),1)
LDFLAGS+=-static
endif

#-------- bwt --------------------------
ifeq ($(BWTSATAN), 1)
CFLAGS+=-D_BWTSATAN
BWT=1
endif

ifeq ($(BWT), 1)
CFLAGS+=-D_BWT
ifeq ($(BWTDIV), 1)
CFLAGS+=-DPROJECT_VERSION_FULL="20137" -DINLINE=inline -Ilibdivsufsort/include -Ilibdivsufsort/build/include 
CFLAGS+=-D_BWTDIV
LIBBWT+=libdivsufsort/unbwt.o 
ifeq ($(NOCOMP), 1)
else
LIBBWT+=libdivsufsort/lib/sssort.o libdivsufsort/lib/utils.o libdivsufsort/lib/daware.o 
LIBBWT+=libdivsufsort/lib/divsufsort.o 
endif
else
ifeq ($(BWTX), 1)
LIBBWT =../bwt/sssort.o ../bwt/bwtxinv.o ../bwt/divsufsort.o ../bwt/trsort.o
CFLAGS+=-D_BWTX
else
CFLAGS+=-D_LIBSAIS -Ilibsais/include
LIBBWT+=$(B)libsais/src/libsais.o
ifeq ($(LIBSAIS16), 1)
CFLAGS+=-D_LIBSAIS16
LIBBWT+=$(B)libsais/src/libsais16.o
endif
endif
endif
endif

all: turborc

ifneq ($(NOCOMP), 1)
LIB=rc_ss.o rc_s.o rccdf.o rcutil.o bec_b.o rccm_s.o rccm_ss.o rcqlfc_s.o rcqlfc_ss.o rcqlfc_sf.o 

#ifeq ($(DELTA), 1)
#CFLAGS+=-D_DELTA
#LIB+=transform.o
#endif

ifeq ($(ANS), 1)
CFLAGS+=-D_ANS
L=./
$(L)anscdf0.o: $(L)anscdf.c $(L)anscdf_.h
	$(CC) -c -O3 $(CFLAGS) -mno-sse2 -falign-loops=32 $(L)anscdf.c -o $(L)anscdf0.o  

$(L)anscdfs.o: $(L)anscdf.c $(L)anscdf_.h
	$(CC) -c -O3 $(CFLAGS) -march=corei7-avx -mtune=corei7-avx -mno-aes -falign-loops=32 $(L)anscdf.c -o $(L)anscdfs.o  

$(L)anscdfx.o: $(L)anscdf.c $(L)anscdf_.h
	$(CC) -c -O3 $(CFLAGS) -march=haswell -falign-loops=32 $(L)anscdf.c -o $(L)anscdfx.o 
LIB+=$(L)anscdfx.o $(L)anscdfs.o $(L)anscdf0.o
endif

ifeq ($(TURBORLE), 1)
CFLAGS+=-D_TURBORLE
LIB+=trlec.o trled.o
endif

ifeq ($(TRANSPOSE), 1)
transpose_avx2.o: transpose.c
	$(CC) -O3 -w -mavx2 $(OPT) -c transpose.c -o transpose_avx2.o

CFLAGS+=-D_TRANSPOSE -D_NCPUISA
LIB+=transpose.o transpose_.o transpose_avx2.o
endif

ifeq ($(V8), 1)
CFLAGS+=-D_V8
LIB+=v8.o
endif
#trlec.o trled.o $(L)anscdfx.o $(L)anscdfs.o $(L)anscdf0.o 
endif
ifeq ($(BWT), 1)
LIB+=rcbwt.o
endif

ifeq ($(SF), 1)
CFLAGS+=-D_SF
LIB+=rc_sf.o rcqlfc_sf.o
endif

ifeq ($(EXT), 1)
CFLAGS+=-D_EXT
#LIB+=xrc.o
ifeq ($(BWT), 1)
#LIB+=xrcbwt_sf.o
endif
endif

ifeq ($(NOCOMP), 1)
CFLAGS+=-DNO_COMP
endif


#librc.a: $(LIB)
#	ar cr $@ $+
turborc.o: turborc.c
	$(CC) -O3 $(CFLAGS) $(MARCH) -c turborc.c -o turborc.o

turborc: $(LIB) $(LIBBWT) turborc.o
	$(CC) $^ $(LDFLAGS) -o turborc

reorder: $(LIBDIV) reorder.o
	$(CC) $^ $(LDFLAGS) -o reorder

.c.o:
	$(CC) -O3 $(CFLAGS)  $(MARCH) $< -c -o $@

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
