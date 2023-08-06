/**
    Copyright (C) powturbo 2013-2023
    GPL v3 License

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    - homepage : https://sites.google.com/site/powturbo/
    - github   : https://github.com/powturbo
    - twitter  : https://twitter.com/powturbo
    - email    : powturbo [_AT_] gmail [_DOT_] com
**/
// TurboRC Range Coder : include header 
#if defined(_MSC_VER) && (_MSC_VER < 1600)
  #if !defined(_STDINT) && !defined(_MSC_STDINT_H_)
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
  #endif
#else
#include <stdint.h>
#endif
#include <stddef.h>
/*#define __STDC_WANT_IEC_60559_TYPES_EXT__
#include <float.h>

#if defined(__clang__) && defined(__is_identifier)
  #if !__is_identifier(_Float16)
    #undef FLT16_BUILTIN
  #endif
#elif defined(FLT16_MAX) || defined(__HAVE_FLOAT16) 
#define FLT16_BUILTIN
#endif*/

#ifdef __cplusplus
extern "C" {
#endif
size_t lzpenc(unsigned char *__restrict in, size_t inlen,  unsigned char *__restrict out, unsigned lenmin, unsigned h_bits);
size_t lzpdec(unsigned char *__restrict in, size_t outlen, unsigned char *__restrict out, unsigned lenmin, unsigned h_bits);

unsigned char *rcqlfc(unsigned char *__restrict in, size_t n, unsigned char *__restrict out, unsigned char *__restrict r2c);

size_t utf8enc(unsigned char *__restrict in, size_t inlen,  unsigned char *__restrict out, unsigned flag);
size_t utf8dec(unsigned char *__restrict in, size_t outlen, unsigned char *__restrict out);
void memrev(unsigned char a[], unsigned n);

//----------- 16 = 2x8,1x16 ----------------
unsigned  delta8l16(uint8_t *in, size_t n);
void      delta8e16(uint8_t *in, size_t n, uint8_t *out);
void      delta8d16(uint8_t *in, size_t n, uint8_t *out);

unsigned delta16l16(uint8_t *in, size_t n);
void     delta16e16(uint8_t *in, size_t n, uint8_t *out);
void     delta16d16(uint8_t *in, size_t n, uint8_t *out);
//----------- 24 = 3x8 -----------------
unsigned  delta8l24(uint8_t *in, size_t n);
void      delta8e24(uint8_t *in, size_t n, uint8_t *out);
void      delta8d24(uint8_t *in, size_t n, uint8_t *out);

//----------- 32 = 8x4,4x16,1x32 ----------------
unsigned  delta8l32(uint8_t *in, size_t n);
void      delta8e32(uint8_t *in, size_t n, uint8_t *out);
void      delta8d32(uint8_t *in, size_t n, uint8_t *out);

unsigned delta16l32(uint8_t *in, size_t n);
void     delta16e32(uint8_t *in, size_t n, uint8_t *out);
void     delta16d32(uint8_t *in, size_t n, uint8_t *out);

unsigned delta32l32(uint8_t *in, size_t n);
void     delta32e32(uint8_t *in, size_t n, uint8_t *out);
void     delta32d32(uint8_t *in, size_t n, uint8_t *out);

void xorenc64(    unsigned char *in, size_t inlen, unsigned char *out);
void xordec64(    unsigned char *in, size_t inlen, unsigned char *out);
void xorenc32(    unsigned char *in, size_t inlen, unsigned char *out);
void xordec32(    unsigned char *in, size_t inlen, unsigned char *out);
void xorenc16(    unsigned char *in, size_t inlen, unsigned char *out);
void xordec16(    unsigned char *in, size_t inlen, unsigned char *out);

void zzagenc16(   unsigned char *in, size_t inlen, unsigned char *out);
void zzagdec16(   unsigned char *in, size_t inlen, unsigned char *out);
void zzagenc32(   unsigned char *in, size_t inlen, unsigned char *out);
void zzagdec32(   unsigned char *in, size_t inlen, unsigned char *out);
void zzagenc64(   unsigned char *in, size_t inlen, unsigned char *out);
void zzagdec64(   unsigned char *in, size_t inlen, unsigned char *out);

//------- Quantization [0..qmax] : qmax maximum quantized value in out ----------------
  #if defined(FLT16_BUILTIN) 
size_t fpquant8e16( _Float16 *in, size_t inlen, uint8_t  *out, unsigned qmax, _Float16 *pfmin, _Float16 *pfmax, _Float16 zmin);
size_t fpquant16e16(_Float16 *in, size_t inlen, uint16_t *out, unsigned qmax, _Float16 *pfmin, _Float16 *pfmax, _Float16 zmin);
//size_t fpquantv8e16(_Float16 *in, size_t inlen, uint8_t  *out, unsigned qmax, _Float16 *pfmin, _Float16 *pfmax, _Float16 zmin);                                                                                                                             
  #endif

size_t fpquant8e32(     float *in, size_t inlen, uint8_t  *out, unsigned qmax, float    *pfmin,    float *pfmax,   float zmin);
size_t fpquant16e32(    float *in, size_t inlen, uint16_t *out, unsigned qmax, float    *pfmin,    float *pfmax,   float zmin);
size_t fpquant32e32(    float *in, size_t inlen, uint32_t *out, unsigned qmax, float    *pfmin,    float *pfmax,   float zmin);

size_t fpquant8e64(    double *in, size_t inlen, uint8_t  *out, unsigned qmax, double   *pfmin,   double *pfmax,  double zmin);
size_t fpquant16e64(   double *in, size_t inlen, uint16_t *out, unsigned qmax, double   *pfmin,   double *pfmax,  double zmin);
size_t fpquant32e64(   double *in, size_t inlen, uint32_t *out, unsigned qmax, double   *pfmin,   double *pfmax,  double zmin);
size_t fpquant64e64(   double *in, size_t inlen, uint64_t *out, unsigned qmax, double   *pfmin,   double *pfmax,  double zmin);

    #if defined(FLT16_BUILTIN) 
size_t fpquant8d16(  uint8_t  *in, size_t outlen, _Float16 *out, unsigned qmax, _Float16   fmin, _Float16   fmax, size_t inlen);
size_t fpquant16d16( uint16_t *in, size_t outlen, _Float16 *out, unsigned qmax, _Float16   fmin, _Float16   fmax);
//size_t fpquantv8d16( uint8_t  *in, size_t outlen, _Float16 *out, unsigned qmax, _Float16   fmin, _Float16   fmax);
    #endif                                                                                                       
																												 
size_t fpquant8d32(  uint8_t  *in, size_t outlen, float    *out, unsigned qmax, float      fmin,    float   fmax, size_t inlen);
size_t fpquant16d32( uint16_t *in, size_t outlen, float    *out, unsigned qmax, float      fmin,    float   fmax);
size_t fpquant32d32( uint32_t *in, size_t outlen, float    *out, unsigned qmax, float      fmin,    float   fmax);
																												 
size_t fpquant8d64(  uint8_t  *in, size_t outlen, double   *out, unsigned qmax, double     fmin,   double   fmax, size_t inlen);
size_t fpquant16d64( uint16_t *in, size_t outlen, double   *out, unsigned qmax, double     fmin,   double   fmax);
size_t fpquant32d64( uint32_t *in, size_t outlen, double   *out, unsigned qmax, double     fmin,   double   fmax);
size_t fpquant64d64( uint64_t *in, size_t outlen, double   *out, unsigned qmax, double     fmin,   double   fmax);

//------- Lossy floating point transform: pad the trailing mantissa bits with zeros according to the error e (ex. e=0.00001)
// must include float.h to use _Float16 (see icapp.c)
 #ifdef FLT16_BUILTIN
_Float16 _fprazor16(_Float16 d, float e, int lg2e);  
void fprazor16(_Float16 *in, unsigned n, _Float16  *out, float  e);
  #endif

float  _fprazor32(float  d, float  e, int lg2e);
double _fprazor64(double d, double e, int lg2e);
void   fprazor32( float  *in, unsigned n, float  *out, float  e);
void   fprazor64(double  *in, unsigned n, double *out, double e);

#ifdef __cplusplus
}
#endif
