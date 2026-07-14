/**
    Copyright (C) powturbo 2015-2026
    SPDX-License-Identifier: GPL v2 License

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

	- email    : powturbo [AT] gmail.com
    - github   : https://github.com/powturbo
    - homepage : https://sites.google.com/site/powturbo/
    - twitter  : https://twitter.com/powturbo
**/

#include <string.h>
#include "include_/conf.h"
#include "include_/cpu.h"
#include "include_/transpose.h"

typedef void (*TPFUNC)( unsigned char *in, unsigned n, unsigned char *out);

                        // 0  1       2       3       4      5  6  7       8   9                    16
static TPFUNC _tpe[]   = { 0, 0, tpenc2,  tpenc3,  tpenc4,   0, 0, 0, tpenc8,  0, 0, 0, 0, 0, 0, 0, tpenc16 }; //  byte
static TPFUNC _tpd[]   = { 0, 0, tpdec2,  tpdec3,  tpdec4,   0, 0, 0, tpdec8,  0, 0, 0, 0, 0, 0, 0, tpdec16 };

static TPFUNC _tp4e[]  = { 0, 0, tpenc2,  tpenc3,  tpenc4,   0, 0, 0, tpenc8,  0, 0, 0, 0, 0, 0, 0, tpenc16 }; // Nibble
static TPFUNC _tp4d[]  = { 0, 0, tpdec2,  tpdec3,  tpdec4,   0, 0, 0, tpdec8,  0, 0, 0, 0, 0, 0, 0, tpdec16 };

//-- zigzag delta
  #ifndef NTP_ZZAG
static TPFUNC _tpze[]  = { 0, 0, tpzenc2, tpzenc3, tpzenc4,  0, 0, 0, tpzenc8, 0, 0, 0, 0, 0, 0, 0, tpzenc16 }; // byte
static TPFUNC _tpzd[]  = { 0, 0, tpzdec2, tpzdec3, tpzdec4,  0, 0, 0, tpzdec8, 0, 0, 0, 0, 0, 0, 0, tpzdec16 };

static TPFUNC _tp4ze[] = { 0, 0, tpzenc2, tpzenc3, tpzenc4,  0, 0, 0, tpzenc8, 0, 0, 0, 0, 0, 0, 0, tpzenc16 }; // Nibble
static TPFUNC _tp4zd[] = { 0, 0, tpzdec2, tpzdec3, tpzdec4,  0, 0, 0, tpzdec8, 0, 0, 0, 0, 0, 0, 0, tpzdec16 };
  #endif

//-- xor
  #ifndef NTP_XOR
static TPFUNC _tpxe[]  = { 0, 0, tpxenc2, tpxenc3, tpxenc4,  0, 0, 0, tpxenc8, 0, 0, 0, 0, 0, 0, 0, tpxenc16 }; // byte
static TPFUNC _tpxd[]  = { 0, 0, tpxdec2, tpxdec3, tpxdec4,  0, 0, 0, tpxdec8, 0, 0, 0, 0, 0, 0, 0, tpxdec16 };

static TPFUNC _tp4xe[] = { 0, 0, tpxenc2, tpxenc3, tpxenc4,  0, 0, 0, tpxenc8, 0, 0, 0, 0, 0, 0, 0, tpxenc16 }; // Nibble
static TPFUNC _tp4xd[] = { 0, 0, tpxdec2, tpxdec3, tpxdec4,  0, 0, 0, tpxdec8, 0, 0, 0, 0, 0, 0, 0, tpxdec16 };
  #endif

static int tpset;

void tpini(int id) {
  int i;
  if(tpset) return;
  tpset++;
  i = id?id:cpuisa();
    #if (defined(__i386__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)) && !defined(_NAVX2)
  if(i >= IS_AVX2) {
    _tpe[ 2] = tpenc256v2;  _tpd[ 2] = tpdec256v2;  _tp4e[ 2] = tp4enc256v2;  _tp4d[ 2] = tp4dec256v2;
    _tpe[ 4] = tpenc256v4;  _tpd[ 4] = tpdec256v4;  _tp4e[ 4] = tp4enc256v4;  _tp4d[ 4] = tp4dec256v4;
    _tpe[ 8] = tpenc256v8;  _tpd[ 8] = tpdec256v8;  _tp4e[ 8] = tp4enc256v8;  _tp4d[ 8] = tp4dec256v8;

      #ifndef NTP_ZZAG
    _tpze[2] = tpzenc256v2; _tpzd[2] = tpzdec256v2; _tp4ze[2] = tp4zenc256v2; _tp4zd[2] = tp4zdec256v2;
    _tpze[4] = tpzenc256v4; _tpzd[4] = tpzdec256v4; _tp4ze[4] = tp4zenc256v4; _tp4zd[4] = tp4zdec256v4;
    _tpze[8] = tpzenc256v8; _tpzd[8] = tpzdec256v8; _tp4ze[8] = tp4zenc256v8; _tp4zd[8] = tp4zdec256v8;
      #endif

      #ifndef NTP_XOR
    _tpxe[2] = tpxenc256v2; _tpxd[2] = tpxdec256v2; _tp4xe[2] = tp4xenc256v2; _tp4xd[2] = tp4xdec256v2;
    _tpxe[4] = tpxenc256v4; _tpxd[4] = tpxdec256v4; _tp4xe[4] = tp4xenc256v4; _tp4xd[4] = tp4xdec256v4;
    _tpxe[8] = tpxenc256v8; _tpxd[8] = tpxdec256v8; _tp4xe[8] = tp4xenc256v8; _tp4xd[8] = tp4xdec256v8;
      #endif
  } else
    #endif
      #if (defined(__i386__) || defined(__x86_64__) || defined(__ARM_NEON) || defined(__riscv_vector) || defined(__powerpc64__) || defined(_M_X64) || defined(_M_IX86)) && !defined(_NSSE)
    if(i >= IS_SSE2) {
      _tpe[ 2] = tpenc128v2;  _tpd[ 2] = tpdec128v2;  _tp4e[ 2] = tp4enc128v2;  _tp4d[ 2] = tp4dec128v2;
      _tpe[ 4] = tpenc128v4;  _tpd[ 4] = tpdec128v4;  _tp4e[ 4] = tp4enc128v4;  _tp4d[ 4] = tp4dec128v4;
      _tpe[ 8] = tpenc128v8;  _tpd[ 8] = tpdec128v8;  _tp4e[ 8] = tp4enc128v8;  _tp4d[ 8] = tp4dec128v8;
      //if(i == 35) _tpd[8] = tpdec8; // ARM NEON scalar is faster!, TODO:retest on Apple M?

       #ifndef NTP_ZZAG
      _tpze[2] = tpzenc128v2; _tpzd[2] = tpzdec128v2; _tp4ze[2] = tp4zenc128v2; _tp4zd[2] = tp4zdec128v2;
      _tpze[4] = tpzenc128v4; _tpzd[4] = tpzdec128v4; _tp4ze[4] = tp4zenc128v4; _tp4zd[4] = tp4zdec128v4;
      _tpze[8] = tpzenc128v8; _tpzd[8] = tpzdec128v8; _tp4ze[8] = tp4zenc128v8; _tp4zd[8] = tp4zdec128v8;
      //if(i == 35) _tpzd[8] = tpzdec8;
        #endif

        #ifndef NTP_XOR
      _tpxe[2] = tpxenc128v2; _tpxd[2] = tpxdec128v2; _tp4xe[2] = tp4xenc128v2; _tp4xd[2] = tp4xdec128v2;
      _tpxe[4] = tpxenc128v4; _tpxd[4] = tpxdec128v4; _tp4xe[4] = tp4xenc128v4; _tp4xd[4] = tp4xdec128v4;
      _tpxe[8] = tpxenc128v8; _tpxd[8] = tpxdec128v8; _tp4xe[8] = tp4xenc128v8; _tp4xd[8] = tp4xdec128v8;
      //if(i == 35) _tpxd[8] = tpxdec8;
        #endif
}
      #endif
  ;
}

void tpenc(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tpe[esize])) f(in,n,out);
  else {
    unsigned i, stride=n/esize;
    unsigned char *op,*ip;
    for(ip = in,op = out; ip < in+stride*esize; op++)
      for(i = 0; i < esize; i++)
        op[i*stride] = *ip++;
    for(op = out + esize*stride; ip < in+n;)
      *op++ = *ip++;
  }
}

void tpdec(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tpd[esize])) f(in,n,out);
  else {
    unsigned i, stride = n/esize;
    unsigned char *op,*ip;
    for(op = out,ip = in; op < out+stride*esize; ip++)
      for(i = 0; i < esize; i++)
        *op++ = ip[i*stride];
    for(ip = in+esize*stride; op < out+n;)
      *op++ = *ip++;
  }
}

void tp4enc(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tp4e[esize])) f(in,n,out);
  else tpenc(in,n,out,esize);
}

void tp4dec(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tp4d[esize])) f(in,n,out);
  else tpdec(in,n,out,esize);
}

//-- zigzag
void tpzenc(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tpze[esize])) f(in,n,out);
  else {
    unsigned i, stride = n/esize;
    unsigned char *op, *ip;
    for(ip = in,op = out; ip < in+stride*esize; op++)
      for(i = 0; i < esize; i++)
        op[i*stride] = *ip++; // TODO:zigzag
    for(op = out + esize*stride; ip < in+n;)
      *op++ = *ip++;          // TODO:zigzag
  }
}

void tpzdec(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tpzd[esize])) f(in,n,out);
  else {
    unsigned i, stride = n/esize;
    unsigned char *op,*ip;
    for(op = out,ip = in; op < out+stride*esize; ip++)
      for(i = 0; i < esize; i++)
        *op++ = ip[i*stride];
    for(ip = in+esize*stride; op < out+n;)
      *op++ = *ip++;
  }
}

void tp4zenc(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tp4ze[esize])) f(in,n,out);
  else tpzenc(in,n,out,esize);
}

void tp4zdec(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tp4zd[esize])) f(in,n,out);
  else tpzdec(in,n,out,esize);
}

//-- xor
void tpxenc(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tpxe[esize])) f(in,n,out);
  else {
    unsigned i, stride = n/esize;
    unsigned char *op, *ip;
    for(ip = in,op = out; ip < in+stride*esize; op++)
      for(i = 0; i < esize; i++)
        op[i*stride] = *ip++; // TODO:xor
    for(op = out + esize*stride; ip < in+n;)
      *op++ = *ip++;          // TODO:xor
  }
}

void tpxdec(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tpxd[esize])) f(in,n,out);
  else {
    unsigned i, stride = n/esize;
    unsigned char *op,*ip;
    for(op = out,ip = in; op < out+stride*esize; ip++)
      for(i = 0; i < esize; i++)
        *op++ = ip[i*stride];
    for(ip = in+esize*stride; op < out+n;)
      *op++ = *ip++;
  }
}

void tp4xenc(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tp4xe[esize])) f(in,n,out);
  else tpxenc(in,n,out,esize);
}

void tp4xdec(unsigned char *in, unsigned n, unsigned char *out, unsigned esize) {
  TPFUNC f;
  if(!tpset) tpini(0);
  if(esize <= 16 && (f = _tp4xd[esize])) f(in,n,out);
  else tpxdec(in,n,out,esize);
}

#define E for(e = esize-1; e >= 0; e--)
#define ODX2 (x + y * nx) * esize + e
void tp2denc(unsigned char *in, unsigned nx, unsigned ny, unsigned char *out, unsigned esize) {
  switch(esize) {
	case 2: tp2denc2(in,nx,ny,out); break;
	case 4: tp2denc4(in,nx,ny,out); break;
	case 8: tp2denc8(in,nx,ny,out); break;
	default: {
	  unsigned x,y;
      uint8_t  *op = out, *ip = in;
      int     e;
      for(  x = 0; x < nx; x++)
        for(y = 0; y < ny; y++) E
          op[ODX2] = *ip++;
    }
  }
}

void tp2ddec(unsigned char *in, unsigned nx, unsigned ny, unsigned char *out, unsigned esize) {
  switch(esize) {
	case 2: tp2ddec2(in,nx,ny,out); break;
	case 4: tp2ddec4(in,nx,ny,out); break;
	case 8: tp2ddec8(in,nx,ny,out); break;
    default: { unsigned x,y;
      uint8_t *op = out, *ip = in;
      int     e;
      for(  x = 0; x < nx; x++)
        for(y = 0; y < ny; y++) E
          *op++ = ip[ODX2];
    }
  }
}
#undef ODX2

#define ODX3 (x + y * nx + z * ny * nx) * esize + e
void tp3denc(unsigned char *in, unsigned nx, unsigned ny, unsigned nz, unsigned char *out, unsigned esize) {
  switch(esize) {
	case 2: tp3denc2(in,nx,ny,nz,out); break;
	case 4: tp3denc4(in,nx,ny,nz,out); break;
	case 8: tp3denc8(in,nx,ny,nz,out); break;
	default: {  unsigned x,y,z;
      uint8_t *op = out, *ip = in;
      int e;
      for(    x = 0; x < nx; x++)
        for(  y = 0; y < ny; y++)
          for(z = 0; z < nz; z++) E
            op[ODX3] = *ip++;
	}
  }
}

void tp3ddec(unsigned char *in, unsigned nx, unsigned ny, unsigned nz, unsigned char *out, unsigned esize) {
  switch(esize) {
	case 2: tp3ddec2(in,nx,ny,nz,out); break;
	case 4: tp3ddec4(in,nx,ny,nz,out); break;
	case 8: tp3ddec8(in,nx,ny,nz,out); break;
    default: {  unsigned x,y,z;
      uint8_t *op = out, *ip = in;
      int e;
      for(x = 0; x < nx; ++x)
        for(y = 0; y < ny; ++y)
          for(z = 0; z < nz; ++z) E
            *op++ = ip[ODX3];
    }
  }
}
#undef ODX3

#define ODX4 (w + x * nw + y * nx * nw + z * nx * ny * nw) * esize + e
void tp4denc(unsigned char *in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *out, unsigned esize) {
  switch(esize) {
	case 2: tp4denc2(in,nw,nx,ny,nz,out); break;
	case 4: tp4denc4(in,nw,nx,ny,nz,out); break;
	case 8: tp4denc8(in,nw,nx,ny,nz,out); break;
	default: {
	  unsigned w, x, y, z;
      uint8_t  *op = out, *ip = in;
      int      e;
      for(      w = 0; w < nw; w++)
        for(    x = 0; x < nx; x++)
          for(  y = 0; y < ny; y++)
            for(z = 0; z < nz; z++) E
              op[ODX4] = *ip++;
	}
  }
}

void tp4ddec(unsigned char *in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *out, unsigned esize) {
  switch(esize) {
	case 2: tp4ddec2(in,nw,nx,ny,nz,out); break;
	case 4: tp4ddec4(in,nw,nx,ny,nz,out); break;
	case 8: tp4ddec8(in,nw,nx,ny,nz,out); break;
    default: {
	  unsigned w,x,y,z;
      uint8_t *op = out,*ip = in;
      int e;
      for(      w = 0; w < nw; w++)
        for(    x = 0; x < nx; ++x)
          for(  y = 0; y < ny; ++y)
            for(z = 0; z < nz; ++z) E
              *op++ = ip[ODX4];
    }
  }
}
