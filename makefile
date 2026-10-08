# powturbo (c) Copyright 2013-2026
# Download or clone TurboRC:
# git clone git://github.com/powturbo/Turbo-Range-Coder.git
BUILD := build
CFLAGS :=
DEFS :=
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
  CFLAGS+=-DHAVE_MALLOC_MALLOC
else ifneq (,$(findstring powerpc64le,$(CC) $(ARCH)))
  ARCH = ppc64le
else ifneq (,$(findstring loongarch64,$(CC) $(ARCH)))
  ARCH = loongarch64
else ifneq (,$(findstring x86_64,$(CC) $(ARCH)))
  ARCH = x86_64
endif

ifeq ($(ARCH),aarch64)
  _SSE=-march=armv8-a
  CFLAGS+=$(_SSE)
else ifeq ($(ARCH),riscv64)
#  CFLAGS+=-march=rv64gcv -mabi=lp64d
  CFLAGS+=-march=rv64gcv_zvbb -mabi=lp64d
else ifeq ($(ARCH),ppc64le)
  _SSE=-D__SSE4_1__
  CFLAGS+=-mcpu=power9 -mtune=power9 $(_SSE)
else ifeq ($(ARCH),loongarch64)
  _SSE=-mlsx
  CFLAGS+=$(_SSE)
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

# ---------- OpenMP detection ----------
HAVE_OPENMP := 0
FOPENMP     :=
OMP_CFLAGS  :=
OMP_LDFLAGS :=
ifneq ($(OPENMP),0) 
  ifeq ($(OS),Darwin)
    LIBOMP_PREFIX := $(shell brew --prefix libomp 2>/dev/null)
    ifneq ($(LIBOMP_PREFIX),)
      FOPENMP     := -Xpreprocessor -fopenmp
      OMP_CFLAGS  := -I$(LIBOMP_PREFIX)/include
      OMP_LDFLAGS := -L$(LIBOMP_PREFIX)/lib -lomp
      ifneq ($(shell echo 'int main(){return 0;}' | $(CC) $(FOPENMP) $(OMP_CFLAGS) $(OMP_LDFLAGS) -x c - -o /dev/null 2>/dev/null && echo ok),)
        HAVE_OPENMP := 1
      endif
    endif
  else ifneq (,$(filter MINGW% MSYS% UCRT% CLANG%,$(MSYSTEM)))
    # Windows / MSYS2 – test whether -fopenmp actually works
    ifeq ($(findstring clang,$(CC)),clang)
      FOPENMP := -fopenmp=libgomp
    else
      FOPENMP := -fopenmp
    endif
    HAVE_OPENMP := $(shell echo 'int main(){return 0;}' | \
    $(CC) $(FOPENMP) -x c - -o /dev/null 2>/dev/null && echo 1 || echo 0)
  else
    # Linux
    ifeq ($(findstring clang,$(CC)),clang)
      FOPENMP := -fopenmp=libgomp
    else
      FOPENMP := -fopenmp
    endif
    HAVE_OPENMP := $(shell echo 'int main(){return 0;}' | $(CC) $(FOPENMP) -x c - -o /dev/null 2>/dev/null && echo 1 || echo 0)
  endif
endif

ifeq ($(HAVE_OPENMP),0)
  $(warning OpenMP not available)
  FOPENMP :=
else
  $(info OpenMP enabled with $(FOPENMP))
  CFLAGS_BWT += -DLIBSAIS_OPENMP $(OMP_CFLAGS)
  CFLAGS     += -DLIBSAIS_OPENMP
  LDFLAGS += $(OMP_LDFLAGS)
endif

#---------------------------------------------------------------
CFLAGS+=$(_SSE) -w -Wall $(DDEBUG) -DBUILD_VERSION="\"v$(BUILD_DATE)\"" $(DEFS)
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

all: $(BUILD)/librc.a $(BUILD)/turborc

ifeq ($(EXTRC), 1)
CFLAGS+=-DEXTRC
endif
#-------- bwt : turbrc works with libdivsufsort or libsais --------------------------
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
LIBBWT = $(BUILD)/sssort.o $(BUILD)/bwtxinv.o $(BUILD)/divsufsort.o $(BUILD)/trsort.o
CFLAGS+=-D_BWTX

else
CFLAGS += -D_LIBSAIS
CFLAGS_BWT += -Ilibsais/include
LIBBWT = $(BUILD)/libsais.o
ifeq ($(LIBSAIS16), 1)
CFLAGS +=-D_LIBSAIS16
LIBBWT += $(BUILD)/libsais16.o
endif
$(LIBBWT): COMP = $(CC)
$(LIBBWT): SIMD = $(_SSE)
$(BUILD)/libsais.o:    libsais/src/libsais.c
$(BUILD)/libsais16.o:  libsais/src/libsais16.c
$(LIBBWT): |  $(BUILD)
	$(COMP) -O3 -falign-loops=32 $(CFLAGS_BWT) $(SIMD) -c $< -o $@
endif

endif
endif




OBJS_CC_SSE := $(BUILD)/bec_b.o $(BUILD)/cpu.o $(BUILD)/rc_ss.o $(BUILD)/rc_s.o $(BUILD)/rccdf.o $(BUILD)/rccmd_s.o $(BUILD)/rccmd_ss.o $(BUILD)/rcqlfc_s.o $(BUILD)/rcqlfc_ss.o 
ifeq ($(BWT), 1)
OBJS_CC_SSE += $(BUILD)/rcbwt.o
endif
ifeq ($(ANS), 1)
CFLAGS+=-D_ANS
OBJS_CX_SSE += $(BUILD)/anscdfs.o
ifeq ($(ARCH), x86_64)
OBJS_CX_AVX2+=$(BUILD)/anscdfx.o
endif
endif
ifeq ($(V8), 1)
CFLAGS+=-D_V8
OBJS_CC_SSE += $(BUILD)/v8.o
endif
ifeq ($(NZ), 1)
CFLAGS+=-D_NZ
OBJS_CC_SSE+=$(BUILD)/rc_nz.o $(BUILD)/rccm_nz.o $(BUILD)/rcqlfc_nz.o
endif
ifeq ($(SH), 1)
CFLAGS+=-D_SH
OBJS_CC_SSE+=$(BUILD)/rc_sh.o
endif
ifeq ($(TURBORLE), 1)
CFLAGS+=-D_TURBORLE
OBJS_CC_SSE += $(BUILD)/trlec.o $(BUILD)/trled.o
endif
$(OBJS_CC_SSE): COMP = $(CC)
$(OBJS_CC_SSE): SIMD = $(_SSE)
$(BUILD)/bec_b.o:      bec_b.c 
$(BUILD)/cpu.o:        cpu.c
$(BUILD)/rc_s.o:       rc_s.c
$(BUILD)/rc_ss.o:      rc_ss.c
$(BUILD)/rcbwt.o:      rcbwt.c
$(BUILD)/rccdf.o:      rccdf.c
$(BUILD)/rccmd_s.o:    rccmd_s.c
$(BUILD)/rccmd_ss.o:   rccmd_ss.c
$(BUILD)/rcqlfc_s.o:   rcqlfc_s.c
$(BUILD)/rcqlfc_ss.o:  rcqlfc_ss.c
$(BUILD)/rcqlfc_sf.o:  rcqlfc_sf.
$(BUILD)/tp.o:         tp.c
$(BUILD)/tp_.o:        tp_.c
$(BUILD)/trlec.o:      trlec.c
$(BUILD)/trled.o:      trled.c

OBJS_CX_SSE := $(BUILD)/rccmc_s.o $(BUILD)/rccmc_ss.o $(BUILD)/anscdfs.o
ifeq ($(TP), 1)
CFLAGS+=-D_TP -D_NCPUISA
OBJS_CX_SSE += $(BUILD)/tp.o $(BUILD)/tp_.o
endif
ifeq ($(SF), 1)
CFLAGS+=-D_SF
OBJS_CX_SSE += $(BUILD)/rc_sf.o $(BUILD)/rccm_sf.o $(BUILD)/rcqlfc_sf.o
ifeq ($(BWT), 1)
OBJS_CX_SSE += $(BUILD)/xrcbwt_sf.o
endif
endif
$(OBJS_CX_SSE): COMP = $(CX)
$(OBJS_CX_SSE): SIMD = $(_SSE)
$(BUILD)/anscdfs.o:    anscdf.c anscdf_.h 
$(BUILD)/rccmc_s.o:    rccmc_s.c
$(BUILD)/rccmc_ss.o:   rccmc_ss.c
$(BUILD)/turborc.o:    turborc.c

OBJS_CX_AVX2 := $(BUILD)/rcutil.o
ifeq ($(ANS), 1)
ifeq ($(ARCH), x86_64)
OBJS_CX_AVX2+=$(BUILD)/anscdfx.o
endif
endif
ifeq ($(TP), 1)
ifeq ($(ARCH),x86_64)
OBJS_CX_AVX2 += $(BUILD)/tp256.o
endif
endif
$(OBJS_CX_AVX2): COMP = $(CX)
$(OBJS_CX_AVX2): SIMD = $(_AVX2)
$(BUILD)/anscdfx.o:    anscdf.c anscdf_.h 
$(BUILD)/tp256.o:      tp.c
$(BUILD)/rcutil.o:     rcutil.c

ALL_OBJS := $(OBJS_CC_SSE) $(OBJS_CX_SSE) $(OBJS_CX_AVX2)
$(ALL_OBJS): |  $(BUILD)
	$(COMP) -O3 -falign-loops=32 $(CFLAGS) $(SIMD) -c $< -o $@

LIB+=$(ALL_OBJS)

ifneq ($(wildcard fasta.c),)
CFLAGS+=-D_FASTA
LIB+=$(BUILD)/fasta.o 
endif

ifeq ($(EXT), 1)
CFLAGS+=-D_EXT
#LIB+=$(BUILD)/xrc.o
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

$(BUILD)/turborc: $(LIB) $(LIBBWT) $(BUILD)/librc.a $(BUILD)/turborc.o
	$(CC) $^ $(LDFLAGS) $(FOPENMP) -o $(BUILD)/turborc

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

