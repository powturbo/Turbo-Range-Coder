# powturbo (c) Copyright 2013-2026
# Download or clone TurboRC:
# git clone git://github.com/powturbo/Turbo-Range-Coder.git
BUILD := build
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
#NZ=1
#SF=1
#TURBORLE=1
TP=1
#----------------------------------------------
CC ?= gcc
#CC ?= clang
CXX ?= g++
CX ?= clang
#CX ?= gcc
#CC = clang

#DEBUG=-DDEBUG -g
DEBUG=-DNDEBUG
JAVA_HOME ?= /usr/lib/jvm/java-8-openjdk-amd64
PREFIX ?= /usr/local
DIRBIN ?= $(PREFIX)/bin
DIRINC ?= $(PREFIX)/include
DIRLIB ?= $(PREFIX)/lib
SRC ?= lib/
BUILD_DATE := $(shell date +%Y%m%d)

# All object files that would normally be built next to their sources are
# instead placed into $(BUILD). VPATH lets make locate the corresponding
# source files in their original (sub)directories.
VPATH := .:libdivsufsort:libdivsufsort/lib:libsais/src:../bwt

#------- OS/ARCH -------------------
ifneq (,$(filter Windows%,$(OS)))
  OS := Windows
  CC=gcc
# CC=clang
# CX=gcc
  CX=clang
  CXX=g++
  ARCH=x86_64
else
  OS := $(shell uname -s)
  ARCH := $(shell uname -m)
endif
#$(info OS="$(OS)")

ifndef CROSS
else
ifeq ($(OS), Windows)
CP=$(CROSS)-unknown-elf
else
CP=$(CROSS)-linux-gnu
endif

CXX:=$(CP)-g++
ifeq ($(CX),clang)
CX=clang --target=$(CP) --sysroot=/usr/$(CP) -fuse-ld=lld
ifeq ($(CC),clang)
CC=$(CX)
else
CC:=$(CP)-gcc
endif
else
CC:=$(CP)-gcc
CX=$(CC)
endif
CROSS=$(CC)
endif

ifneq (,$(or $(findstring aarch64,$(CC) $(ARCH)),$(findstring arm64,$(CC) $(ARCH))))
  ARCH = aarch64
else ifneq (,$(findstring riscv64,$(CC) $(ARCH)))
  ARCH = riscv64
else ifneq (,$(findstring iPhone,$(ARCH)))
  ARCH = aarch64
  CFLAGS=-DHAVE_MALLOC_MALLOC
else ifneq (,$(findstring powerpc64le,$(CC) $(ARCH)))
  ARCH = ppc64le
else ifneq (,$(findstring loongarch64,$(CC) $(ARCH)))
  ARCH = loongarch64
else ifneq (,$(findstring x86_64,$(CC) $(ARCH)))
  ARCH = x86_64
endif

ifeq ($(ARCH),aarch64)
  _SSE=-march=armv8-a
  CFLAGS=$(_SSE)
else ifeq ($(ARCH),riscv64)
#  CFLAGS=-march=rv64gcv -mabi=lp64d
  CFLAGS=-march=rv64gcv_zvbb -mabi=lp64d
else ifeq ($(ARCH),ppc64le)
  _SSE=-D__SSE4_1__
  CFLAGS=-mcpu=power9 -mtune=power9 $(_SSE)
else ifeq ($(ARCH),loongarch64)
  _SSE=-mlsx
  CFLAGS=$(_SSE)
else ifeq ($(ARCH),x86_64)
# _SSE=-mssse3 
# _SSE+=-mno-avx -mno-aes
# _SSE=-march=corei7-avx -mtune=corei7-avx
# _SSE=-march=ivybridge -mavx
  _SSE=-mavx -mpopcnt

# _AVX2=-march=skylake-avx512 -mavx512vbmi -mavx512f -mavx512vl
  _AVX2=-march=haswell
endif

ifeq ($(OS),Windows)
  LDFLAGS=-Wl,--stack,33554432 -lpowrprof
endif

CFLAGS+=$(_SSE) -w -Wall $(DDEBUG) -DBUILD_VERSION="\"v$(BUILD_DATE)\""
CXXFLAGS+=$(DDEBUG) -w -Wall -fpermissive  -fno-rtti

ifeq ($(OS),$(filter $(OS),Linux GNU/kFreeBSD GNU OpenBSD FreeBSD DragonFly NetBSD MSYS_NT Haiku))
LDFLAGS+=-lrt -lpthread
endif

ifdef STATIC
LDFLAGS+=-static
endif

#-pedantic
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

all: $(BUILD)/librc.a turborc

ifeq ($(EXTRC), 1)
CFLAGS+=-DEXTRC
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
LIBBWT+=$(BUILD)/unbwt.o 
ifeq ($(NOCOMP), 1)
else
LIBBWT+=$(BUILD)/sssort.o $(BUILD)/utils.o $(BUILD)/daware.o 
LIBBWT+=$(BUILD)/divsufsort.o 
endif
else
ifeq ($(BWTX), 1)
LIBBWT =$(BUILD)/sssort.o $(BUILD)/bwtxinv.o $(BUILD)/divsufsort.o $(BUILD)/trsort.o
CFLAGS+=-D_BWTX
else
CFLAGS+=-D_LIBSAIS -Ilibsais/include
LIBBWT+=$(BUILD)/libsais.o
ifeq ($(LIBSAIS16), 1)
CFLAGS+=-D_LIBSAIS16
LIBBWT+=$(BUILD)/libsais16.o
endif
endif
endif
endif

ifneq ($(NOCOMP), 1)
LIB=$(addprefix $(BUILD)/,rc_ss.o rc_s.o rccdf.o rcutil.o bec_b.o rccm_s.o rccm_ss.o rcqlfc_s.o rcqlfc_ss.o rcqlfc_sf.o cpu.o)


#ifeq ($(DELTA), 1)
#CFLAGS+=-D_DELTA
#LIB+=$(BUILD)/transform.o
#endif

ifeq ($(ANS), 1)
CFLAGS+=-D_ANS
$(BUILD)/anscdf0.o: anscdf.c anscdf_.h | $(BUILD)
	$(CC) -c -O3 $(CFLAGS) $(_SCALAR) -falign-loops=32 anscdf.c -o $(BUILD)/anscdf0.o  

$(BUILD)/anscdfs.o: anscdf.c anscdf_.h | $(BUILD)
	$(CC) -c -O3 $(CFLAGS) $(_SSE) -falign-loops=32 anscdf.c -o $(BUILD)/anscdfs.o  

LIB+=$(BUILD)/anscdfs.o 
ifeq ($(ARCH), x86_64)
$(BUILD)/anscdfx.o: anscdf.c anscdf_.h | $(BUILD)
	$(CC) -c -O3 $(CFLAGS) -march=haswell -falign-loops=32 anscdf.c -o $(BUILD)/anscdfx.o

LIB+=$(BUILD)/anscdfx.o 
#$(BUILD)/anscdf0.o
else
CFLAGS+=-D_NAVX2
endif
endif

ifeq ($(TURBORLE), 1)
CFLAGS+=-D_TURBORLE
LIB+=$(addprefix $(BUILD)/,trlec.o trled.o)
endif

ifeq ($(TP), 1)
ifeq ($(ARCH),x86_64)
$(BUILD)/tp256.o: tp.c | $(BUILD)
	$(CC) -O3 $(CFLAGS) $(_AVX2) -c tp.c -o $(BUILD)/tp256.o

$(BUILD)/rcutil.o: rcutil.c | $(BUILD)
	$(CC) -O3 $(CFLAGS) $(_AVX2) -c rcutil.c -o $(BUILD)/rcutil.o

#tp_.c: tp_.c
#	$(CC) -O3 $(CFLAGS) $(_SSE) -c tp_.c -o tp_.c
	
endif
CFLAGS+=-D_TP -D_NCPUISA
LIB+=$(addprefix $(BUILD)/,tp.o tp_.o)

ifeq ($(ARCH), x86_64)
LIB+=$(BUILD)/tp256.o
endif
endif

ifeq ($(V8), 1)
CFLAGS+=-D_V8
LIB+=$(BUILD)/v8.o
endif
#trlec.o trled.o anscdfx.o anscdfs.o anscdf0.o 
endif
ifeq ($(BWT), 1)
LIB+=$(BUILD)/rcbwt.o
endif

ifeq ($(SF), 1)
CFLAGS+=-D_SF
LIB+=$(addprefix $(BUILD)/,rc_sf.o rccm_sf.o rcqlfc_sf.o)
endif

ifeq ($(NZ), 1)
LIB+=$(addprefix $(BUILD)/,rc_nz.o rccm_nz.o rcqlfc_nz.o)
CFLAGS+=-D_NZ
endif

ifeq ($(SH), 1)
LIB+=$(BUILD)/rc_sh.o
CFLAGS+=-D_SH
endif

ifeq ($(EXT), 1)
CFLAGS+=-D_EXT
#LIB+=$(BUILD)/xrc.o
ifeq ($(BWT), 1)
#LIB+=$(BUILD)/xrcbwt_sf.o
endif
endif

ifeq ($(NOCOMP), 1)
CFLAGS+=-DNO_COMP
endif

$(BUILD)/librc.a: $(LIB) | $(BUILD)
	$(AR) rcs $@ $^

$(BUILD)/librc.so: $(LIB) | $(BUILD)
	$(CC) -shared $+ -o $@

$(BUILD)/turborc.o: turborc.c | $(BUILD)
	$(CC) -O3 $(CFLAGS) $(MARCH) -c turborc.c -o $(BUILD)/turborc.o

turborc: $(LIB) $(LIBBWT) $(BUILD)/librc.a $(BUILD)/turborc.o
	$(CC) $^ $(LDFLAGS) -o $(BUILD)/turborc

reorder: $(LIBDIV) $(BUILD)/reorder.o
	$(CC) $^ $(LDFLAGS) -o $(BUILD)/reorder

# Directory creation
$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) -O3 $(CFLAGS) $(MARCH) $< -c -o $@

$(BUILD)/%.o: %.cpp | $(BUILD)
	$(CXX) -O3 $(MARCH) $(CXXFLAGS) $< -c -o $@ 

ifeq ($(OS),Windows_NT)
clean:
	del /S *.o
	del /S *~
	if exist $(BUILD) rd /S /Q $(BUILD)
else
clean:
	@find . -type f -name "*\.o" -delete -or -name "*\~" -delete -or -name "core" -delete -or -name "librc.a" -delete
	@rm -rf $(BUILD)
endif
