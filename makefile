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
CXX ?= clang++
CXL = clang++
CG ?= clang
CO := $(CG)

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
# CG=gcc
  CG=clang
  CXX=g++
  CXL=clang++
  ARCH=x86_64
else
  OS := $(shell uname -s)
  ARCH := $(shell uname -m)
endif

ifndef CROSS
else

ifeq ($(OS), Windows)
CP=$(CROSS)-unknown-elf
else
CP=$(CROSS)-linux-gnu
endif
ifeq ($(CG),clang)
CG=clang --target=$(CP) --sysroot=/usr/$(CP) -fuse-ld=lld
CXX:=$(CP)-clang++
ifeq ($(CC),clang)
CC=$(CG)
else
CC:=$(CP)-gcc
CXX:=$(CP)-g++
endif
else
CC:=$(CP)-gcc
CG=$(CC)
endif
#CROSS:=$(CC)
endif
CHOST=$(CROSS)

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
else ifeq ($(ARCH),riscv64)
#  CFLAGS=-march=rv64gc_zba_zbb_zbs
#  CFLAGS=-mabi=lp64d -mno-strict-align
  CFLAGS=-march=rv64gc -mabi=lp64d -fno-tree-vectorize  
#-fno-tree-loop-vectorize
  _SSE=-march=rv64gcv_zvbb
else ifeq ($(ARCH),ppc64le)
  _SSE=-D__SSE4_1__
  CFLAGS=-mcpu=power9 -mtune=power9 $(_SSE)
  CHOST=powerpc64le
else ifeq ($(ARCH),loongarch64)
  _SSE=-mlsx
else ifeq ($(ARCH),x86_64)
  _SSE=-mavx -mpopcnt
  _AVX2=-march=haswell
endif

ifeq ($(OS),Windows)
  LDFLAGS=-Wl,--stack,33554432 -lpowrprof
endif

# ---------- OpenMP detection ----------
HAVE_OPENMP := 0
ifneq ($(OPENMP),0) 
  ifeq ($(OS),Darwin)
    LIBOMP_PREFIX := $(shell brew --prefix libomp 2>/dev/null)
    ifneq ($(LIBOMP_PREFIX),)
      OMP_CFLAGS  := -Xpreprocessor -fopenmp -I$(LIBOMP_PREFIX)/include
      OMP_LDFLAGS := -L$(LIBOMP_PREFIX)/lib -lomp
      ifneq ($(shell echo 'int main(){return 0;}' | $(CC) $(OMP_CFLAGS) $(OMP_LDFLAGS) -x c - -o /dev/null 2>/dev/null && echo ok),)
        HAVE_OPENMP := 1
      endif
    endif
  else ifneq (,$(filter MINGW% MSYS% UCRT% CLANG%,$(MSYSTEM)))
    # Windows / MSYS2 – test whether -fopenmp actually works
    OMP_CFLAGS := -fopenmp
    ifeq ($(findstring clang,$(CC)),clang)
      OMP_LDFLAGS := -lomp
    else
      OMP_LDFLAGS := -lgomp
    endif
    HAVE_OPENMP := $(shell \
      echo '#include <omp.h>' > _omp_test.c && \
      echo 'int main(){return omp_get_max_threads();}' >> _omp_test.c && \
      $(CC) $(OMP_CFLAGS) _omp_test.c -o _omp_test $(OMP_LDFLAGS) 2>/dev/null && \
      echo 1 || echo 0; \
      rm -f _omp_test.c _omp_test _omp_test.exe)
  else
    # Linux
    OMP_CFLAGS  := -fopenmp     
    OMP_LDFLAGS := -fopenmp
    HAVE_OPENMP := $(shell echo 'int main(){return 0;}' | $(CO) $(OMP_CFLAGS) -x c - -o /dev/null 2>/dev/null && echo 1 || echo 0)
  endif
endif

ifeq ($(HAVE_OPENMP),0)
  $(warning OpenMP not available)
  OMP_CFLAGS  :=
  OMP_LDFLAGS :=
else
  $(info OpenMP enabled with $(OMP_CFLAGS) and $(OMP_LDFLAGS))
  LDFLAGS += $(OMP_LDFLAGS)
endif

#---------------------------------------------------------------
CFLAGS+=-w $(DDEBUG) -falign-loops=32 
CXXFLAGS+=$(DDEBUG) -w -Wall -fpermissive  -fno-rtti
RC_FLAGS = $(DEFS)

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
RC_FLAGS+=-DEXTRC
endif
#-------- bwt : turbrc works with libdivsufsort or libsais --------------------------
ifeq ($(BWTSATAN), 1)
RC_FLAGS+=-D_BWTSATAN
BWT=1
endif

ifeq ($(BWT), 1)
RC_FLAGS+=-D_BWT
ifeq ($(BWTDIV), 1)
RC_FLAGS+=-DPROJECT_VERSION_FULL="20137" -DINLINE=inline -Ilibdivsufsort/include -Ilibdivsufsort/build/include 
RC_FLAGS+=-D_BWTDIV
LIBBWT+=$(BUILD)/unbwt.o 
ifeq ($(NOCOMP), 1)
else
LIBBWT+=$(BUILD)/sssort.o $(BUILD)/utils.o $(BUILD)/daware.o 
LIBBWT+=$(BUILD)/divsufsort.o 
endif
else
ifeq ($(BWTX), 1)
LIBBWT = $(BUILD)/sssort.o $(BUILD)/bwtxinv.o $(BUILD)/divsufsort.o $(BUILD)/trsort.o
RC_FLAGS+=-D_BWTX

else
RC_FLAGS += -D_LIBSAIS
OMP_CFLAGS += -Ilibsais/include 
LIBBWT = $(BUILD)/libsais.o
ifeq ($(LIBSAIS16), 1)
RC_FLAGS +=-D_LIBSAIS16
LIBBWT += $(BUILD)/libsais16.o
endif

ifeq ($(HAVE_OPENMP),1)
$(LIBBWT): COMP = $(CO) -DLIBSAIS_OPENMP
else
$(LIBBWT): COMP = $(CO) 
endif

ifeq ($(ARCH), x86_64)
$(LIBBWT): SIMD = $(_AVX2)
else
$(LIBBWT): SIMD = $(_SSE)
endif

$(BUILD)/libsais.o:    libsais/src/libsais.c
$(BUILD)/libsais16.o:  libsais/src/libsais16.c
$(LIBBWT): |  $(BUILD)
	$(COMP) -O3 $(CFLAGS) $(OMP_CFLAGS) $(SIMD) -c $< -o $@
#-Wno-deprecated-openmp 
endif

endif
endif

OBJS_CC_SSE := $(BUILD)/bec_b.o $(BUILD)/cpu.o $(BUILD)/rc_ss.o $(BUILD)/rc_s.o $(BUILD)/rccdf.o $(BUILD)/rccmd_s.o $(BUILD)/rccmd_ss.o 
ifeq ($(ANS), 1)
RC_FLAGS+=-D_ANS
OBJS_CG_SSE += $(BUILD)/anscdfs.o
ifeq ($(ARCH), x86_64)
OBJS_CG_AVX2+=$(BUILD)/anscdfx.o
endif
endif
ifeq ($(V8), 1)
RC_FLAGS+=-D_V8
OBJS_CC_SSE += $(BUILD)/v8.o
endif
ifeq ($(NZ), 1)
RC_FLAGS+=-D_NZ
OBJS_CC_SSE+=$(BUILD)/rc_nz.o $(BUILD)/rccm_nz.o $(BUILD)/rcqlfc_nz.o
endif
ifeq ($(SH), 1)
RC_FLAGS+=-D_SH
OBJS_CC_SSE+=$(BUILD)/rc_sh.o
endif
ifeq ($(TURBORLE), 1)
RC_FLAGS+=-D_TURBORLE
OBJS_CC_SSE += $(BUILD)/trlec.o $(BUILD)/trled.o
endif
$(OBJS_CC_SSE): COMP = $(CC)
$(OBJS_CC_SSE): SIMD = $(_SSE)
$(BUILD)/bec_b.o:      bec_b.c 
$(BUILD)/cpu.o:        cpu.c
$(BUILD)/rc_s.o:       rc_s.c
$(BUILD)/rc_ss.o:      rc_ss.c
$(BUILD)/rccdf.o:      rccdf.c
$(BUILD)/rccmd_s.o:    rccmd_s.c
$(BUILD)/rccmd_ss.o:   rccmd_ss.c
$(BUILD)/tp.o:         tp.c
$(BUILD)/tp_.o:        tp_.c
$(BUILD)/trlec.o:      trlec.c
$(BUILD)/trled.o:      trled.c

OBJS_CG_SSE := $(BUILD)/rccmc_s.o $(BUILD)/rccmc_ss.o $(BUILD)/anscdfs.o
ifeq ($(TP), 1)
RC_FLAGS+=-D_TP -D_NCPUISA
OBJS_CG_SSE += $(BUILD)/tp.o $(BUILD)/tp_.o
endif
ifeq ($(SF), 1)
RC_FLAGS+=-D_SF
OBJS_CG_SSE += $(BUILD)/rc_sf.o $(BUILD)/rccm_sf.o
ifeq ($(BWT), 1)
OBJS_CG_SSE += $(BUILD)/xrcbwt_sf.o
endif
endif
ifeq ($(BWT), 1)
OBJS_CG_SSE += $(BUILD)/rcbwt.o
endif

$(OBJS_CG_SSE): COMP = $(CG) $(OMP_CFLAGS)
$(OBJS_CG_SSE): SIMD = $(_SSE)
$(BUILD)/anscdfs.o:    anscdf.c anscdf_.h 
$(BUILD)/rccmc_s.o:    rccmc_s.c
$(BUILD)/rccmc_ss.o:   rccmc_ss.c
$(BUILD)/rcbwt.o:      rcbwt.c

OBJS_CG_AVX2 := $(BUILD)/rcutil.o $(BUILD)/rcqlfc_s.o $(BUILD)/rcqlfc_ss.o  $(BUILD)/rcqlfc_sf.o
ifeq ($(ANS), 1)
ifeq ($(ARCH), x86_64)
OBJS_CG_AVX2+=$(BUILD)/anscdfx.o
endif
endif
ifeq ($(TP), 1)
ifeq ($(ARCH),x86_64)
OBJS_CG_AVX2 += $(BUILD)/tp256.o
endif
endif
$(OBJS_CG_AVX2): COMP = $(CG) -DBUILD_VERSION="\"v$(BUILD_DATE)\"" 
$(OBJS_CG_AVX2): SIMD = $(_AVX2)
$(BUILD)/anscdfx.o:    anscdf.c anscdf_.h 
$(BUILD)/tp256.o:      tp.c
$(BUILD)/rcutil.o:     rcutil.c
$(BUILD)/rcqlfc_s.o:   rcqlfc_s.c
$(BUILD)/rcqlfc_ss.o:  rcqlfc_ss.c
$(BUILD)/rcqlfc_sf.o:  rcqlfc_sf.c

ALL_OBJS := $(OBJS_CC_SSE) $(OBJS_CG_SSE) $(OBJS_CG_AVX2)
$(ALL_OBJS): |  $(BUILD)
	$(COMP) -O3 $(CFLAGS) $(SIMD) -c $< -o $@

LIB+=$(ALL_OBJS)

ifneq ($(wildcard fasta.c),)
RC_FLAGS+=-D_FASTA
LIB+=$(BUILD)/fasta.o 
endif

ifeq ($(EXT), 1)
RC_FLAGS+=-D_EXT
#LIB+=$(BUILD)/xrc.o
endif

ifeq ($(NOCOMP), 1)
RC_FLAGS+=-DNO_COMP
endif

$(BUILD)/librc.a: $(LIB) | $(BUILD)
	$(AR) rcs $@ $^

$(BUILD)/librc.so: $(LIB) | $(BUILD)
	$(CC) -shared $+ -o $@

$(BUILD)/turborc.o: turborc.c | $(BUILD)
	$(CG) -O3 $(CFLAGS) $(MARCH) $(RC_FLAGS) -c turborc.c -o $(BUILD)/turborc.o

$(BUILD)/turborc: $(LIB) $(LIBBWT) $(BUILD)/librc.a $(BUILD)/turborc.o
	$(CXL) $^ $(LDFLAGS) -o $(BUILD)/turborc

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

