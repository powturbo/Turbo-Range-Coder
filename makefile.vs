# Copyright (C) powturbo 2015-2022
# nmake "NCODEC1=1" "NCODEC2=1" /f makefile.msc
# or
# nmake "AVX2=1" /f makefile.msc

.SUFFIXES: .c .obj .dllobj

CC = cl /nologo
LD = link /nologo
AR = lib /nologo
CFLAGS = /MD -I.
LDFLAGS = 

LIB_LIB = libttrc.lib
LIB_DLL = turborc.dll
LIB_IMP = turborc.lib

OBJS	= rc_ss.obj rc_s.obj rccdf.obj rccm_s.obj rccm_ss.obj rcutil.obj bec_b.obj 

!IF "$(AVX2)" == "1"
DEFS = $(DEFS) /D__AVX2__
!endif

!IF "$(NSSE2)" != "1"
DEFS = $(DEFS) /D__SSE__
!endif

DLL_OBJS = $(OBJS:.obj=.dllobj)

all:	$(LIB_LIB) turborc.exe 

#$(LIB_DLL) $(LIB_IMP) 

.c.obj:
	$(CC) -c /Fo$@ /O2 $(CFLAGS) $(DEFS) $**

.cc.obj:
	$(CC) -c /Fo$@ /O2 $(CFLAGS) $(DEFS) $**

.c.dllobj:
	$(CC) -c /Fo$@ /O2 $(CFLAGS) $(DEFS) /DLIB_DLL $**

$(LIB_LIB): $(OBJS)
	$(AR) $(ARFLAGS) -out:$@ $(OBJS)

$(LIB_DLL): $(DLL_OBJS)
	$(LD) $(LDFLAGS) -out:$@ -dll -implib:$(LIB_IMP) $(DLL_OBJS)

$(LIB_IMP): $(LIB_DLL)

turborc.exe: turborc.obj vs/getopt.obj $(LIB_LIB)
	$(LD) $(LDFLAGS) -out:$@ $**

clean:
	-del *.dll *.exe *.exp *.obj *.dllobj *.lib *.manifest 2>nul

